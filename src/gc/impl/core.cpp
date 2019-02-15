
/*
ISC License (ISC)

Copyright 2018-2019 Adam Armstrong

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

/**	\file src/gc/impl/core.cpp
 *	\brief Implements the GC core.
 */

#include "core.hpp"

#include "soft_ptr_data.hpp"

namespace gc {

collector::collector() noexcept :
	mtx(), alive(true), readers(), writer(), flag(false), count(0), worker(), requests(0),
	sources(), traversable(),
	period(0 < config::gc_period ? std::chrono::milliseconds{config::gc_period} :
								   std::chrono::milliseconds::zero())
{}

collector::~collector() noexcept
{
	SYNAFIS_ASSERT(managed.empty());
	SYNAFIS_ASSERT(unmanaged.empty());
	alive = false;
	{
		std::lock_guard<std::mutex> l{mtx};
		flag = false;
		SYNAFIS_ASSERT(count == 0);
	}
	writer.notify_one();
	worker.join();
	{
		std::lock_guard<std::mutex> l{mtx};
		flag = true;
	}
	readers.notify_all();
}

void collector::init_impl()
{
	std::lock_guard<std::mutex> l{mtx};
	allocators.shrink_to_fit(); // Ideally all allocators are setup at this point.
	worker = std::thread{[this]() -> void { this->work(); }};
	flag = true;
}

void collector::lock_impl()
{
	std::unique_lock<std::mutex> l{mtx};
	readers.wait(l, [this]() -> bool { return this->wait_read(); });
	count++;
}

void collector::unlock_impl()
{
	std::unique_lock<std::mutex> l{mtx};
	count--;
	if (!flag && count == 0) {
		l.unlock();
		writer.notify_one();
	}
}

void collector::wait_impl(std::unique_lock<std::mutex> &l)
{
	flag = false;
	count--;
	requests++;
	l.unlock();
	writer.notify_one();
	l.lock();
	readers.wait(l, [this]() -> bool { return this->wait_read(); });
	count++;
}

template<typename T1, typename T2, typename F>
void collector::insert_helper(
	std::vector<T1> &vec, T2 const &find, T1 &&add, F &func, std::unique_lock<std::mutex> &l)
{
	auto const it = std::upper_bound(vec.cbegin(), vec.cend(), find, func);
	try {
		vec.insert(it, std::forward<T1>(add));
		return;
	} catch (std::bad_alloc const &) {}
	wait_impl(l);
	vec.insert(it, std::forward<T1>(add));
}

void collector::register_root_impl(void *obj, traverse_cb tcb, root_cb rcb)
{
	std::unique_lock<std::mutex> l{mtx};
	source const src{find_source_impl(obj)};
	if (src) {
		insert_helper(managed, obj, mroot{obj, src},
			[](void *addr, auto const &cur) -> bool { return addr < cur.obj; }, l);
		SYNAFIS_ASSERT(std::is_sorted(managed.cbegin(), managed.cend(),
			[](auto const &x, auto const &y) -> bool { return x.obj < y.obj; }));
	} else {
		insert_helper(unmanaged, obj, uroot{obj, tcb, rcb},
			[](void *addr, auto const &cur) -> bool { return addr < cur.obj; }, l);
		SYNAFIS_ASSERT(std::is_sorted(unmanaged.cbegin(), unmanaged.cend(),
			[](auto const &x, auto const &y) -> bool { return x.obj < y.obj; }));
	}
}

void collector::unregister_root_impl(void *obj) noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	auto const m = std::lower_bound(managed.cbegin(), managed.cend(), obj,
		[](auto const &cur, void *addr) -> bool { return cur.obj < addr; });
	if (m == managed.cend() || m->obj != obj) {
		auto const u = std::lower_bound(unmanaged.cbegin(), unmanaged.cend(), obj,
			[](auto const &cur, void *addr) -> bool { return cur.obj < addr; });
		if (u != unmanaged.cend() && u->obj == obj) { unmanaged.erase(u); }
	} else {
		managed.erase(m);
	}
}

isource *collector::find_source_impl(void *ptr) const noexcept
{
	auto const upper = std::upper_bound(sources.cbegin(), sources.cend(), ptr,
		[](void *p, auto const &cur) -> bool { return p < cur->location(); });
	if (upper == sources.cbegin()) {
		return nullptr;
	} else {
		source const it{*(upper - 1)};
		if (it->from(ptr)) {
			return it;
		} else {
			return nullptr;
		}
	}
}

void *collector::base_ptr_impl(void *ptr) const noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	source const from{find_source_impl(ptr)};
	return from ? from->base_of(ptr) : nullptr;
}

identity const *collector::get_type_impl(void *ptr) const noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	source const from{find_source_impl(ptr)};
	return from ? from->type_of(ptr) : nullptr;
}

void collector::insert_source(isource &src, bool trav) noexcept
{
	std::unique_lock<std::mutex> l{mtx};
	insert_helper(sources, src.location(), std::addressof(src),
		[](void *addr, auto const &cur) -> bool { return addr < cur->location(); }, l);
	SYNAFIS_ASSERT(std::is_sorted(sources.cbegin(), sources.cend(),
		[](auto const &x, auto const &y) -> bool { return x->location() < y->location(); }));
	if (trav) {
		insert_helper(traversable, src.location(), std::addressof(src),
			[](void *addr, auto const &cur) -> bool { return addr < cur->location(); }, l);
		SYNAFIS_ASSERT(std::is_sorted(traversable.cbegin(), traversable.cend(),
			[](auto const &x, auto const &y) -> bool { return x->location() < y->location(); }));
	}
}

void collector::erase_source(
	std::vector<source> &vec, decltype(vec.cbegin()) &start, isource const &src) noexcept
{
	auto const it = std::lower_bound(start, vec.cend(), src.location(),
		[](source cur, void *addr) -> bool { return cur->location() < addr; });
	if (it != vec.cend()) {
		SYNAFIS_ASSERT(std::addressof(src) == *it);
		start = vec.erase(it);
	}
}

collector::iallocator *collector::insert_alloc_impl(alloc_ptr &&alloc) noexcept
{
	iallocator *ptr{alloc.get()};
	std::unique_lock<std::mutex> l{mtx};
	insert_helper(allocators, ptr, std::forward<alloc_ptr>(alloc),
		[](iallocator *find, alloc_ptr const &cur) -> bool { return find < cur.get(); }, l);
	SYNAFIS_ASSERT(std::is_sorted(allocators.cbegin(), allocators.cend(),
		[](auto const &x, auto const &y) -> bool { return x < y; }));
	return ptr;
}

void collector::erase_alloc_impl(iallocator const &alloc) noexcept
{
	if (alive.load()) {
		// The allocator's destructor needs to wait on running until mtx is not held.
		alloc_ptr ptr{nullptr};
		{
			std::lock_guard<std::mutex> l{mtx};
			auto it = std::lower_bound(allocators.begin(), allocators.end(), std::addressof(alloc),
				[](alloc_ptr const &cur, iallocator const *addr) -> bool {
					return addr <= cur.get();
				});
			if (it != allocators.end()) {
				ptr = std::move(*it);
				allocators.erase(it);
			}
		}
	}
}

void collector::work() noexcept
{
	std::unique_lock<std::mutex> l{mtx, std::defer_lock};
	do {
		l.lock();
		if (period == duration::zero()) {
			writer.wait(l, [this]() -> bool { return !flag; });
		} else {
			if (!writer.wait_for(l, period, [this]() -> bool { return !flag; })) { flag = false; }
		}
		writer.wait(l, [this]() -> bool { return count < 1; });
		mark();
		sweep();
		shrink();
		flag = true;
		l.unlock();
		readers.notify_all();
	} while (alive.load());
}

void collector::enumerate(void *data, void *ptr) noexcept
{
	SYNAFIS_ASSERT(data != nullptr);
	SYNAFIS_ASSERT(ptr != nullptr);
	collector *const ref{reinterpret_cast<collector *>(data)};
	source const src{ref->find_source_impl(ptr)};
	SYNAFIS_ASSERT(src != nullptr);
	src->mark(ptr);
}

void collector::mark() noexcept
{
	for (uroot const &x : unmanaged) {
		x.tcb(x.obj, this, enumerate);
	}
	for (mroot const &x : managed) {
		x.src->mark(x.obj);
	}
	bool found;
	do {
		found = false;
		for (source const cur : traversable) {
			found |= cur->traverse(this, enumerate);
		}
	} while (found);
}

void collector::sweep() noexcept
{
	for (source cur : sources) {
		cur->sweep();
	}
}

void collector::shrink() noexcept
{
	if (0 < requests) {
		do {
			std::size_t freed{0};
			std::size_t average{(requests / allocators.size()) + 1};
			for (alloc_ptr &cur : allocators) {
				freed += cur->shrink(average);
			}
			if (0 < freed) {
				requests = (requests <= freed ? 0 : requests - freed);
			} else {
				requests = 0;
				return;
			}
		} while (0 < requests);
	} else {
		for (alloc_ptr &cur : allocators) {
			cur->shrink(0);
		}
	}
}

bool collector::wait_read() const
{
	if (alive.load()) {
		return flag;
	} else {
		throw std::runtime_error{
			"A thread was waiting after the collector's destructor was called."};
	}
}

collector &collector::singleton() noexcept
{
	static collector the_one_and_only{};
	return the_one_and_only;
}

}
