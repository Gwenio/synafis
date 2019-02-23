
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

/**	\file test/gc/gc.cpp
 *	\brief Constructs the GC test suites in the correct order.
 */

#include "gc.hpp"

namespace gc_test {

extern suite collector_suite{"collector", gc_suite};

extern suite alloc_suite{"allocator", gc_suite};

extern suite pool_suite{"pool", gc_suite};

extern suite lock_suite{"lock", gc_suite};

extern suite vmem_suite{"vmem", gc_suite};

extern suite ptrmath_suite{"pointer math", gc_suite};

}

namespace gc {

//!	\cond impl_details

using gc_test::simple;

using gc_test::simple_ptr;

template<>
identity const &get_id<simple>() noexcept
{
	static identity const id{static_cast<simple *>(nullptr)};
	return id;
}

template<>
identity const &get_id<simple_ptr>() noexcept
{
	static identity const id{static_cast<simple_ptr *>(nullptr)};
	return id;
}

template identity const &get_id<simple>() noexcept;

template identity const &get_id<simple_ptr>() noexcept;

//!	\endcond

}
