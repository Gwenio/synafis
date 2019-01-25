
/*
ISC License (ISC)

Copyright 2019 Adam Armstrong

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

#include "../../src/gc/identity.hpp"

#ifndef SYNAFIS_TEST_GC_HPP
#define SYNAFIS_TEST_GC_HPP
#pragma once

/**	\file test/gc/gc.hpp
 *	\brief Declares the test suites for the garbage collector.
 */

#include "../master.hpp"
#include "../../src/gc/callbacks.hpp"
#include "../../src/gc/traits.hpp"

namespace unit_test {

/**	\namespace gc
 *	\brief Namespace for GC unit test items.
 */
namespace gc {

// Declare the gc test suites.

/**	\var collector_suite
 *	\brief The test suite for ::gc::collector.
 */
extern suite collector_suite;

/**	\var alloc_suite
 *	\brief The test suite for ::gc::allocator.
 */
extern suite alloc_suite;

/**	\var pool_suite
 *	\brief The test suite for ::gc::pool.
 */
extern suite pool_suite;

/**	\var lock_suite
 *	\brief The test suite for ::gc::basic_mutex and ::gc::debug_mutex.
 */
extern suite lock_suite;

/**	\var vmem_suite
 *	\brief The test suite for ::gc::vmem.
 */
extern suite vmem_suite;

/**	\var ptrmath_suite
 *	\brief The test suite for the pointer math functions.
 */
extern suite ptrmath_suite;

//!	\cond impl_details

/**	\class simple
 *	\brief A simple type for allocator testing.
 */
class simple
{
public:
	simple() = default;
	~simple() = default;
	std::uintptr_t data;
};

/**	\class simple_ptr
 *	\brief A simple type for allocator testing containing a pointer.
 */
class simple_ptr
{
public:
	simple_ptr() noexcept = default;
	~simple_ptr() noexcept = default;
	simple_ptr(simple_ptr const &) noexcept = delete;
	simple_ptr(simple_ptr &&other) : data(std::exchange(other.data, nullptr)) {}
	simple_ptr &operator=(simple_ptr const &) = delete;
	simple_ptr &operator=(simple_ptr &&other) noexcept
	{
		data = std::exchange(other.data, nullptr);
		return *this;
	}
	void traverse(void *arg, ::gc::enumerate_cb cb) const noexcept { cb(arg, data); }
	void remap(void *arg, ::gc::remap_cb cb) noexcept
	{
		data = static_cast<simple *>(cb(arg, data));
	}
	simple *data;
};

//!	\endcond

}

}

namespace gc {

//!	\cond impl_details

namespace traits {

template<>
class pointers_type<unit_test::gc::simple_ptr> : public std::integral_constant<bool, true>
{};

}

extern template identity const &get_id<unit_test::gc::simple>() noexcept;

extern template identity const &get_id<unit_test::gc::simple_ptr>() noexcept;

//!	\endcond

}

#endif
