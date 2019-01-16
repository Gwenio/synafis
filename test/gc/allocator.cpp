
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

/**	\file test/gc/allocator.cpp
 *	\brief Defines test cases for gc::allocator.
 */

#include "allocator.hpp"
#include "../../src/gc/idaccess.hpp"
#include "gc.hpp"

#include <algorithm>
#include <stdexcept>
#include <array>

namespace {

using namespace gc;

using simple = unit_test::gc::simple;

constexpr inline static std::size_t const simple_unit{
	std::max(idaccess::unit_size<simple>(), pool::min_unit)};

constexpr inline static auto const simple_flags = traits::get_flags<simple>();

using t = unit_test::tester<allocator>;

}

namespace unit_test {

//!	\cond impl_details

bool t::invariants(allocator const &obj) noexcept
{
	if (obj.pools.empty()) {
		return false;
	} else {
		for (handle const &x : obj.pools) {
			if (!x) {
				SYNAFIS_FAILURE("A pool was not allocated.");
				return false;
			}
		}
		for (auto cur = obj.full_begin; cur != obj.pools.cend(); cur++) {
			if (!cur->full()) {
				SYNAFIS_FAILURE("A pool in the full range is not full.");
				return false;
			}
		}
		SYNAFIS_ASSERT(std::is_sorted(
			obj.full_begin, obj.pools.cend(), [](handle const &x, handle const &y) -> bool {
				return std::addressof(*x) < std::addressof(*y);
			}));
		return true;
	}
}

void t::creation(collector &)
{
	allocator temp{get_id<simple>(), simple_unit, simple_flags};
	SYNAFIS_ASSERT(invariants(temp));
}

void t::growth(collector &)
{
	allocator temp{get_id<simple>(), simple_unit, simple_flags};
	SYNAFIS_ASSERT(invariants(temp));
	auto &h = temp.grow();
	SYNAFIS_ASSERT(std::addressof(h) == std::addressof(temp.pools.front()));
	SYNAFIS_ASSERT(invariants(temp));
	if (temp.pools.size() < 2) {
		SYNAFIS_FAILURE("The allocator contained too few pools.");
	} else if (temp.pools.size() > 2) {
		SYNAFIS_FAILURE("The allocator contained too many pools.");
	}
}

//!	\endcond

}

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s{unit_test::gc::alloc_suite};

static c growth{"growth", s, pass, &t::growth};

static c creation{"creation", s, pass, &t::creation};

}
