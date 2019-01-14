
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

#include "../../src/gc/identity.hpp"

#ifndef SYNAFIS_TEST_GC_HPP
#define SYNAFIS_TEST_GC_HPP
#pragma once

/**	\file test/gc/gc.hpp
 *	\brief Declares the test suites for the garbage collector.
 */

#include "../master.hpp"

namespace unit_test {

/**	\namespace gc
 *	\brief Namespace for GC unit test items.
 */
namespace gc {

// Declare the gc test suites.

/**	\var alloc_suite
 *	\brief The main test suite for the garbage collector.
 */
extern suite alloc_suite;

/**	\var pool_suite
 *	\brief The main test suite for the garbage collector.
 */
extern suite pool_suite;

/**	\var ptrmath_suite
 *	\brief The main test suite for the garbage collector.
 */
extern suite ptrmath_suite;

/**	\var vmem_suite
 *	\brief The main test suite for the garbage collector.
 */
extern suite vmem_suite;

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
	simple_ptr() = default;
	~simple_ptr() = default;
	simple *data;
};

}

}

namespace gc {

extern template identity const &get_id<unit_test::gc::simple>() noexcept;

extern template identity const &get_id<unit_test::gc::simple_ptr>() noexcept;

}

#endif
