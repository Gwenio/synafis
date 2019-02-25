
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

/**	\file test/gc/pool/gray_list.hpp
 *	\brief Defines test cases for gc::gray_list.
 */

#include "gray_list.hpp"

#include "../../../src/gc/impl/pool/gray_list.hpp"
#include "suites.hpp"

#include <array>

using gc::gray_list;

//!	\cond impl_details

using t = typename unit_test::tester<gray_list>;
using utc = typename unit_test::collector;

void t::invariants(gray_list const &obj) noexcept { SYNAFIS_ASSERT(obj.sentinel <= obj.current); }

void t::null_list(utc &)
{
	//!	TEST that gc::gray_list initializes correctly with a nullptr.
	gray_list temp{nullptr};
	SYNAFIS_ASSERT(temp.sentinel == nullptr);
	SYNAFIS_ASSERT(temp.current == nullptr);
	SYNAFIS_ASSERT(!static_cast<bool>(temp));
	SYNAFIS_ASSERT(temp.pending() == 0);
	SYNAFIS_ASSERT(!temp.has_pending());
}

void t::creation(utc &)
{
	//!	TEST that gc::gray_list initializes correctly with a valid pointer.
	std::array<void *, 4> buffer{nullptr};
	gray_list temp{buffer.data()};
	SYNAFIS_ASSERT(temp.sentinel == buffer.data());
	SYNAFIS_ASSERT(temp.current == buffer.data());
	SYNAFIS_ASSERT(static_cast<bool>(temp));
	SYNAFIS_ASSERT(temp.pending() == 0);
	SYNAFIS_ASSERT(!temp.has_pending());
}

void t::pushing(utc &)
{
	std::array<void *, 4> buffer{nullptr};
	gray_list temp{buffer.data()};
	std::array<std::size_t, 4> region{{0, 1, 2, 3}};
	//!	TEST that gc::gray_list pushes new values correctly.
	temp.push(std::addressof(region[0]));
	SYNAFIS_ASSERT(temp.has_pending());
	invariants(temp);
	SYNAFIS_ASSERT(temp.pending() == 1);
	SYNAFIS_ASSERT(temp.sentinel[1] == nullptr);
	temp.push(std::addressof(region[3]));
	SYNAFIS_ASSERT(temp.pending() == 2);
	SYNAFIS_ASSERT(temp.sentinel[2] == nullptr);
	temp.push(std::addressof(region[2]));
	SYNAFIS_ASSERT(temp.pending() == 3);
	SYNAFIS_ASSERT(temp.sentinel[3] == nullptr);
	temp.push(std::addressof(region[1]));
	SYNAFIS_ASSERT(temp.pending() == 4);
	SYNAFIS_ASSERT(temp.has_pending());
	invariants(temp);
	SYNAFIS_ASSERT(temp.sentinel[0] == std::addressof(region[0]));
	SYNAFIS_ASSERT(temp.sentinel[1] == std::addressof(region[3]));
	SYNAFIS_ASSERT(temp.sentinel[2] == std::addressof(region[2]));
	SYNAFIS_ASSERT(temp.sentinel[3] == std::addressof(region[1]));
}

void t::popping(utc &)
{
	std::array<void *, 4> buffer{nullptr};
	gray_list temp{buffer.data()};
	std::array<std::size_t, 4> region{{0, 1, 2, 3}};
	void *ptr{nullptr};
	//!	TEST that gc::gray_list pops values correctly.
	temp.push(std::addressof(region[0]));
	ptr = temp.pop();
	SYNAFIS_ASSERT(temp.pending() == 0);
	SYNAFIS_ASSERT(!temp.has_pending());
	SYNAFIS_ASSERT(temp.sentinel == temp.current);
	SYNAFIS_ASSERT(ptr == std::addressof(region[0]));
	temp.push(std::addressof(region[2]));
	temp.push(std::addressof(region[0]));
	temp.push(std::addressof(region[3]));
	ptr = temp.pop();
	SYNAFIS_ASSERT(temp.pending() == 2);
	SYNAFIS_ASSERT(temp.has_pending());
	invariants(temp);
	SYNAFIS_ASSERT(ptr == std::addressof(region[3]));
	temp.push(std::addressof(region[1]));
	temp.push(std::addressof(region[3]));
	SYNAFIS_ASSERT(temp.pending() == 4);
	ptr = temp.pop();
	SYNAFIS_ASSERT(ptr == std::addressof(region[3]));
	SYNAFIS_ASSERT(temp.pending() == 3);
	ptr = temp.pop();
	SYNAFIS_ASSERT(ptr == std::addressof(region[1]));
	SYNAFIS_ASSERT(temp.pending() == 2);
	ptr = temp.pop();
	SYNAFIS_ASSERT(ptr == std::addressof(region[0]));
	SYNAFIS_ASSERT(temp.pending() == 1);
	ptr = temp.pop();
	SYNAFIS_ASSERT(ptr == std::addressof(region[2]));
	SYNAFIS_ASSERT(temp.pending() == 0);
	SYNAFIS_ASSERT(!temp.has_pending());
	SYNAFIS_ASSERT(temp.sentinel == temp.current);
}

//!	\endcond

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s() noexcept { return gc_test::gray_list_suite; }

c popping{"popping", s(), pass, &t::popping};

c pushing{"pushing", s(), pass, &t::pushing};

c creation{"creation", s(), pass, &t::creation};

c null_list{"null list", s(), pass, &t::null_list};

}
