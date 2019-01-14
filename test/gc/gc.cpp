
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

/**	\file test/gc/gc.cpp
 *	\brief Constructs the GC test suites in the correct order.
 */

#include "gc.hpp"

namespace unit_test {

namespace gc {

extern suite alloc_suite{"allocator", gc_suite};

extern suite pool_suite{"pool", gc_suite};

extern suite ptrmath_suite{"pointer math", gc_suite};

extern suite vmem_suite{"vmem", gc_suite};

}

}

namespace gc {

using unit_test::gc::simple;

using unit_test::gc::simple_ptr;

static identity const simple_id(static_cast<simple *>(nullptr));

static identity const simple_ptr_id(static_cast<simple_ptr *>(nullptr));

template<>
identity const &get_id<simple>() noexcept
{
	return simple_id;
}

template<>
identity const &get_id<simple_ptr>() noexcept
{
	return simple_ptr_id;
}

template identity const &get_id<simple>() noexcept;

template identity const &get_id<simple_ptr>() noexcept;

}
