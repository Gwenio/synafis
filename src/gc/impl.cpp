
/*
ISC License (ISC)

Copyright 2018 Adam Armstrong

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

#include "impl.hpp"

#include "soft_ptr_data.hpp"

namespace gc {

collector::collector() noexcept :
	mtx(), readers(), writer(), flag(false), count(0), worker(), alive(), requests(0),
	period(std::chrono::milliseconds{config::gc_period}), sources()
{
	alive.test_and_set();
}

collector::~collector() noexcept
{
	alive.clear();
	worker.join();
}

void collector::init_impl()
{
	std::lock_guard<std::mutex> l{mtx};
	worker = std::thread{[this]() -> void { this->work(); }};
	flag = true;
}

void collector::lock_impl()
{
	std::unique_lock<std::mutex> l{mtx};
	readers.wait(l, [this]() -> bool { return this->flag; });
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
	readers.wait(l, [this]() -> bool { return this->flag; });
	count++;
}

soft_ptr::data *collector::get_soft_ptr_impl(void *ptr) { return nullptr; }

void collector::free_soft_ptr_impl(soft_ptr::data *ptr) {}

template<typename T1, typename T2, typename F>
void collector::insert_helper(
	std::vector<T1> &vec, T2 const &find, T1 const add, F &func, std::unique_lock<std::mutex> &l)
{
	auto const it = std::upper_bound(vec.cbegin(), vec.cend(), find, func);
	try {
		vec.insert(it, add);
		return;
	} catch (std::bad_alloc const &) {}
	wait_impl(l);
	vec.insert(it, add);
}

void collector::register_root_impl(void *obj, traverse_cb tcb, root_cb rcb)
{
	std::unique_lock<std::mutex> l{mtx};
	source const src{find_source(obj)};
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
		[](auto const &cur, void *addr) -> bool { return addr <= cur.obj; });
	if (m == managed.cend() || m->obj != obj) {
		auto const u = std::lower_bound(unmanaged.cbegin(), unmanaged.cend(), obj,
			[](auto const &cur, void *addr) -> bool { return addr <= cur.obj; });
		if (u != unmanaged.cend() && u->obj == obj) { unmanaged.erase(u); }
	} else {
		managed.erase(m);
	}
}

isource *collector::find_source(void *ptr) const noexcept
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
	source const from{find_source(ptr)};
	return from ? from->base_of(ptr) : nullptr;
}

identity const *collector::get_type_impl(void *ptr) const noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	source const from{find_source(ptr)};
	return from ? from->type_of(ptr) : nullptr;
}

void collector::insert_source_impl(isource &src) noexcept
{
	std::unique_lock<std::mutex> l{mtx};
	insert_helper(sources, src.location(), std::addressof(src),
		[](void *addr, auto const &cur) -> bool { return addr < cur->location(); }, l);
	SYNAFIS_ASSERT(std::is_sorted(sources.cbegin(), sources.cend(),
		[](auto const &x, auto const &y) -> bool { return x->location() < y->location(); }));
}

void collector::erase_source_impl(isource &src) noexcept
{
	auto const it = std::lower_bound(sources.cbegin(), sources.cend(), std::addressof(src),
		[](source cur, source addr) -> bool { return addr <= cur; });
	if (it != sources.cend()) { sources.erase(it); }
}

void collector::insert_alloc_impl(iallocator &alloc) noexcept
{
	std::unique_lock<std::mutex> l{mtx};
	iallocator *addr{std::addressof(alloc)};
	insert_helper(allocators, addr, addr,
		[](iallocator *find, iallocator *cur) -> bool { return find < cur; }, l);
	SYNAFIS_ASSERT(std::is_sorted(allocators.cbegin(), allocators.cend(),
		[](auto const &x, auto const &y) -> bool { return x < y; }));
}

void collector::erase_alloc_impl(iallocator &alloc) noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	auto const it = std::lower_bound(allocators.cbegin(), allocators.cend(), std::addressof(alloc),
		[](iallocator *cur, iallocator *addr) -> bool { return addr <= cur; });
	if (it != allocators.cend()) { allocators.erase(it); }
}

void collector::work() noexcept
{
	std::unique_lock<std::mutex> l{mtx, std::defer_lock};
	do {
		l.lock();
		if (!writer.wait_for(l, period, [this]() -> bool { return !flag; })) { flag = false; }
		writer.wait(l, [this]() -> bool { return 0 < count; });
		mark();
		sweep();
		shrink();
		flag = true;
		l.unlock();
		readers.notify_all();
	} while (alive.test_and_set());
}

void collector::mark() noexcept {}

void collector::sweep() noexcept
{
	for (auto cur : sources) {
		cur->sweep();
	}
}

void collector::shrink() noexcept
{
	if (0 < requests) {
		std::size_t remaining{allocators.size()};
		for (iallocator *cur : allocators) {
			std::size_t const freed{cur->shrink((requests / remaining) + 1)};
			requests = requests <= freed ? 0 : requests - freed;
		}
		if (0 < requests) { // If more requests, shrink again going in reverse order.
			for (remaining = allocators.size(); 0 < remaining;) {
				std::size_t const goal{(requests / remaining) + 1};
				std::size_t const freed{allocators[--remaining]->shrink(goal)};
				requests = requests <= freed ? 0 : requests - freed;
			}
			requests = 0;
		}
	} else {
		for (iallocator *cur : allocators) {
			cur->shrink(0);
		}
	}
}

collector collector::singleton{};

}
