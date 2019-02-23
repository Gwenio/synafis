
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

/**	\file test/gc/ptr_math.cpp
 *	\brief Defines test cases for the pointer math functions.
 */

#include "gc.hpp"
#include "../../src/gc/impl/ptr_math.hpp"

namespace {

using unit_test::collector;
using gc::add_offset;
using gc::sub_offset;
using gc::sub_addr;
using std::uintptr_t;
using std::ptrdiff_t;

static_assert(sizeof(void *) == sizeof(std::uintptr_t),
	"The size of void* and std::uintptr_t must be the same for pointer arithmetic to function "
	"correctly.");

void adding(collector &)
{
	SYNAFIS_ASSERT(add_offset(nullptr, 1024) == reinterpret_cast<void *>(1024));
}

void subtract(collector &)
{
	SYNAFIS_ASSERT(sub_offset(reinterpret_cast<void *>(1024), 1024) == nullptr);
}

void difference(collector &)
{
	SYNAFIS_ASSERT(sub_addr(reinterpret_cast<void *>(1024), reinterpret_cast<void *>(1024)) == 0);
	SYNAFIS_ASSERT(sub_addr(reinterpret_cast<void *>(512), reinterpret_cast<void *>(1024)) == -512);
	SYNAFIS_ASSERT(sub_addr(reinterpret_cast<void *>(1024), reinterpret_cast<void *>(512)) == 512);
	SYNAFIS_ASSERT(sub_addr(nullptr, reinterpret_cast<void *>(1024)) == -1024);
	SYNAFIS_ASSERT(sub_addr(reinterpret_cast<void *>(1024), nullptr) == 1024);
}

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s() noexcept { return gc_test::ptrmath_suite; }

c differencing{"difference", s(), pass, difference};

c subtraction{"subtract", s(), pass, subtract};

c addition{"adding", s(), pass, adding};

}
