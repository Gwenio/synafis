
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

/**	\file src/gc/impl/soft_ptr.cpp
 *	\brief Implements portions of gc::soft_ptr that depend on GC internals.
 *	\ingroup gc_impl
 */

#include "../soft_ptr.hpp"
#include "../hard_ptr.hpp"
#include "soft_ptr_data.hpp"
#include "core.hpp"

namespace gc {

soft_ptr::data *soft_ptr::get_soft(hard_ptr const &other)
{
	return copy(data::get_soft_ptr(other.ptr));
}

soft_ptr::data *soft_ptr::copy(soft_ptr::data *other) noexcept
{
	data *temp{other->next.load(std::memory_order_relaxed)};
	if (temp) {
		return copy(temp);
	} else if (other->ptr) {
		other->count.fetch_add(1, std::memory_order_relaxed);
		return other;
	} else {
		return nullptr;
	}
}

soft_ptr::data *soft_ptr::update(soft_ptr::data *old) noexcept
{
	data *temp{old->next.load(std::memory_order_relaxed)};
	if (temp) {
		soft_ptr::free(old);
		return copy(temp);
	} else if (old->ptr) {
		return old;
	} else {
		soft_ptr::free(old);
		return nullptr;
	}
}

void soft_ptr::free(soft_ptr::data *other) noexcept
{
	std::size_t temp{other->count.fetch_sub(1, std::memory_order_seq_cst)};
	if (temp == 1) { // temp == 1 means it is now zero
		data::free_soft_ptr(other);
	}
}

soft_ptr::data *soft_ptr::data::create(void *ptr) noexcept
{
	try {
		return new data{ptr};
	} catch (std::bad_alloc const &) {}
	collector::wait();
	return new data{ptr};
}

soft_ptr::data *soft_ptr::data::get_soft_ptr(void *ptr) noexcept
{
	SYNAFIS_ASSERT(ptr != nullptr);
	isource *found{collector::find_source(ptr)};
	SYNAFIS_ASSERT(found);
	return found->fetch(ptr);
}

void soft_ptr::data::insert_stale(soft_ptr::data *ptr) noexcept
{
	SYNAFIS_ASSERT(ptr != nullptr);
	std::size_t temp{ptr->count.fetch_sub(1, std::memory_order_seq_cst)};
	if (temp == 1) { // temp == 1 means it is now zero
		delete ptr;
	} else {
		std::lock_guard<std::mutex> l{mtx};
		auto const it = std::upper_bound(stale.cbegin(), stale.cend(), ptr,
			[](data *addr, data *cur) -> bool { return addr < cur; });
		try {
			stale.insert(it, ptr);
			SYNAFIS_ASSERT(std::is_sorted(
				stale.cbegin(), stale.cend(), [](auto *x, auto *y) -> bool { return x < y; }));
			return;
		} catch (std::bad_alloc const &) {}
		collector::wait();
		stale.insert(it, ptr);
		SYNAFIS_ASSERT(std::is_sorted(
			stale.cbegin(), stale.cend(), [](auto *x, auto *y) -> bool { return x < y; }));
	}
}

void soft_ptr::data::free_soft_ptr(soft_ptr::data *ptr) noexcept
{
	SYNAFIS_ASSERT(ptr != nullptr);
	SYNAFIS_ASSERT(ptr->count.load() == 0);
	std::lock_guard<std::mutex> l{mtx};
	auto const it = std::lower_bound(stale.cbegin(), stale.cend(), ptr,
		[](data *cur, data *addr) -> bool { return cur < addr; });
	if (it != stale.cend()) {
		SYNAFIS_ASSERT(ptr == *it);
		stale.erase(it);
		delete ptr;
	}
}

std::mutex soft_ptr::data::mtx{};

std::vector<soft_ptr::data *> soft_ptr::data::stale{};

}
