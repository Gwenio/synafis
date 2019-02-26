
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
 *	\ingroup gc_impl
 */

#include "core.hpp"

#include "soft_ptr_data.hpp"

namespace {

// If insert fails due to lack of memory, waits for a GC cycle and retries.
template<typename Insert, typename Wait>
void insert_helper(Insert &insert, Wait &wait)
{
	try {
		insert();
		return;
	} catch (std::bad_alloc const &) {}
	wait();
	insert();
}

}

namespace gc {

collector::collector() noexcept :
	mtx(), alive(true), readers(), writer(), flag(false), count(0), worker(), requests(0),
	sources(), traversable(), roots(),
	period(0 < config::gc_period ? std::chrono::milliseconds{config::gc_period} :
								   std::chrono::milliseconds::zero())
{}

collector::~collector() noexcept
{
	SYNAFIS_ASSERT(roots.empty());
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
	worker = std::thread{[this]() -> void { work(); }};
	flag = true;
}

void collector::lock_impl()
{
	std::unique_lock<std::mutex> l{mtx};
	readers.wait(l, [this]() -> bool { return wait_read(); });
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
	readers.wait(l, [this]() -> bool { return wait_read(); });
	count++;
}

void collector::register_root_impl(void *obj, traverse_cb tcb, root_cb rcb)
{
	std::unique_lock<std::mutex> l{mtx};
	SYNAFIS_ASSERT(find_source_impl(obj) == nullptr);
	auto temp = std::make_pair(obj, root_type{tcb, rcb});
	insert_helper(
		[this, &temp]() { roots.emplace(std::move(temp)); }, [this, &l]() { wait_impl(l); });
}

void collector::unregister_root_impl(void *obj) noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	roots.erase(obj);
}

isource *collector::find_source_impl(void *ptr) const noexcept
{
	auto upper = sources.upper_bound(ptr);
	if (upper == sources.end()) {
		return nullptr;
	} else {
		upper--;
		isource *const it{std::get<1>(*upper)};
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
	isource const *from{find_source_impl(ptr)};
	return from ? from->base_of(ptr) : nullptr;
}

identity const *collector::get_type_impl(void *ptr) const noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	isource const *from{find_source_impl(ptr)};
	return from ? from->type_of(ptr) : nullptr;
}

void collector::insert_source(isource &src, bool trav) noexcept
{
	std::unique_lock<std::mutex> l{mtx};
	auto waiter = [this, &l]() { wait_impl(l); };
	insert_helper(
		[this, &src]() { sources.emplace(std::make_pair(src.location(), std::addressof(src))); },
		waiter);
	if (trav) {
		insert_helper([this, &src]() { traversable.emplace(std::addressof(src)); }, waiter);
	}
}

void collector::erase_source(isource const &src, bool trav) noexcept
{
	sources.erase(src.location());
	if (trav) {
		isource *const temp{const_cast<isource *>(std::addressof(src))};
		traversable.erase(temp);
	}
}

collector::iallocator *collector::insert_alloc_impl(alloc_ptr &&alloc) noexcept
{
	iallocator *const ptr{alloc.get()};
	std::unique_lock<std::mutex> l{mtx};
	insert_helper([this, &alloc]() { allocators.emplace_back(std::forward<alloc_ptr>(alloc)); },
		[this, &l]() { wait_impl(l); });
	return ptr;
}

void collector::work() noexcept
{
	std::unique_lock<std::mutex> l{mtx, std::defer_lock};
	auto zero_wait = [this]() -> bool { return !flag; };
	auto timed_wait = [this]() -> bool { return !flag; };
	auto secondary_wait = [this]() -> bool { return count < 1; };
	do {
		l.lock();
		if (period == duration::zero()) {
			writer.wait(l, zero_wait);
		} else {
			if (!writer.wait_for(l, period, timed_wait)) { flag = false; }
		}
		writer.wait(l, secondary_wait);
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
	isource *const src{ref->find_source_impl(ptr)};
	SYNAFIS_ASSERT(src != nullptr);
	src->mark(ptr);
}

void collector::mark() noexcept
{
	for (auto const &x : roots) {
		std::get<1>(x).tcb(std::get<0>(x), this, enumerate);
	}
	bool found;
	do {
		found = false;
		for (isource *const cur : traversable) {
			found |= cur->traverse(this, enumerate);
		}
	} while (found);
}

void collector::sweep() noexcept
{
	for (auto const &cur : sources) {
		std::get<1>(cur)->sweep();
	}
}

void collector::shrink() noexcept
{
	if (0 < requests) {
		do {
			std::size_t freed{0};
			std::size_t average{(requests / allocators.size()) + 1};
			for (alloc_ptr const &cur : allocators) {
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
		for (alloc_ptr const &cur : allocators) {
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
