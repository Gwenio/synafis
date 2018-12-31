
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
	mtx(), readers(), writer(), flag(false), count(0), worker(), alive(),
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
	l.unlock();
	writer.notify_one();
	l.lock();
	readers.wait(l, [this]() -> bool { return this->flag; });
	count++;
}

soft_ptr::data *collector::get_soft_ptr_impl(void *ptr) { return nullptr; }

void collector::free_soft_ptr_impl(soft_ptr::data *ptr) {}

void *collector::base_ptr_impl(void *ptr) const noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	auto const upper = std::upper_bound(sources.cbegin(), sources.cend(), ptr,
		[](void *p, auto const &cur) -> bool { return p < cur->location(); });
	if (upper == sources.cbegin()) {
		return nullptr;
	} else {
		source const it{*(upper - 1)};
		if (it->from(ptr)) {
			return it->base_of(ptr);
		} else {
			return nullptr;
		}
	}
}

void collector::insert_source_impl(isource &src) noexcept
{
	std::unique_lock<std::mutex> l{mtx};
	auto const it = std::upper_bound(sources.cbegin(), sources.cend(), src.location(),
		[](void *addr, auto const &cur) -> bool { return addr < cur->location(); });
	try {
		sources.insert(it, std::addressof(src));
	} catch (std::bad_alloc const &) {
		wait_impl(l);							 // Wait on GC cycle for more memory.
		sources.insert(it, std::addressof(src)); // Try again, crash if it fails.
	}
	SYNAFIS_ASSERT(std::is_sorted(sources.cbegin(), sources.cend(),
		[](auto const &x, auto const &y) -> bool { return x->location() < y->location(); }));
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

collector collector::singleton{};

}
