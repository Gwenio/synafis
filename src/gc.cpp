
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

#include "gc.hpp"

/**	\file src/gc.cpp
 *	\brief Defines the implementation for parts of the GC API.
 */

#include "gc/soft_ptr_data.hpp"
#include "gc/allocator.hpp"
#include "gc/impl.hpp"

namespace gc {

identity::~identity() {}

std::unique_ptr<identity::iallocator> identity::select_alloc(
	identity const &id, std::size_t unit, traits::flag_type flags)
{
	return std::unique_ptr<iallocator>(dynamic_cast<iallocator *>(new allocator(id, unit, flags)));
}

identity const *identity::fetch_impl(void *obj) noexcept { return collector::get_type(obj); }

/**	\var lock_count
 *	\brief Count how many times a thread has acquired the collector's lock.
 */
thread_local std::size_t lock_count{0};

scoped_lock::scoped_lock()
{
	if (lock_count++ == 0) { collector::lock(); }
}

scoped_lock::~scoped_lock()
{
	if (--lock_count == 0) { collector::unlock(); }
}

soft_ptr::data *soft_ptr::get_soft(hard_ptr const &other)
{
	return copy(collector::get_soft_ptr(other.ptr));
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
	if ((!(other->ptr) || other->next) && temp == 1) { // temp == 1 means it is now zero
		collector::free_soft_ptr(other);
	}
}

std::tuple<void *, identity const *> hard_ptr::get_hard(soft_ptr const &other)
{
	using rtype = std::tuple<void *, identity const *>;
	if (other.ptr && other.ptr->ptr) {
		return rtype{other.ptr->ptr, identity::fetch(other.ptr->ptr, std::nothrow)};
	} else {
		return rtype{nullptr, nullptr};
	}
}

void *hard_ptr::base_ptr(void *source) noexcept
{
	return source ? collector::base_ptr(source) : static_cast<void *>(nullptr);
}

void root::register_impl(void *obj, traverse_cb tcb, root_cb rcb)
{
	SYNAFIS_ASSERT(obj != nullptr);
	SYNAFIS_ASSERT(tcb != nullptr);
	SYNAFIS_ASSERT(rcb != nullptr);
	;
}

void root::unregister_impl(void *obj) noexcept
{
	SYNAFIS_ASSERT(obj != nullptr);
	;
}

void initialize() { collector::init(); }

void collect() noexcept { collector::collect(); }

void set_period(std::chrono::steady_clock::duration value) noexcept
{
	collector::set_period(value);
}

}
