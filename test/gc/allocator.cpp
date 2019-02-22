
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
#include "../../src/gc/impl/idaccess.hpp"
#include "gc.hpp"

#include <algorithm>
#include <stdexcept>
#include <array>

using gc::allocator;
using gc::pool;
using gc::blueprint;
using gc::get_id;
using gc::idaccess;
namespace traits = gc::traits;

using t = unit_test::tester<allocator>;

namespace {

using simple = unit_test::gc::simple;

constexpr inline static std::size_t const simple_unit{
	std::max(idaccess::unit_size<simple>(), blueprint::min_unit())};

constexpr inline static auto const simple_flags = traits::get_flags<simple>();

using pool_list = allocator::pool_list;

/**	\fn is_sorted_pools(pool_list const &l)
 *	\brief Checks if a list of pools are sorted.
 *	\param l The list to check.
 *	\returns Returns true if the pools in 'l' are sorted.
 */
bool is_sorted_pools(pool_list const &l)
{
	return std::is_sorted(l.cbegin(), l.cend(), [](pool const &x, pool const &y) { return x < y; });
}

}

namespace unit_test {

//!	\cond impl_details

bool t::invariants(allocator const &obj) noexcept
{
	if (!(obj.empty_pools.empty() || obj.part_pools.empty() || obj.full_pools.empty())) {
		return false;
	} else {
		SYNAFIS_ASSERT(is_sorted_pools(obj.empty_pools));
		SYNAFIS_ASSERT(is_sorted_pools(obj.part_pools));
		SYNAFIS_ASSERT(is_sorted_pools(obj.full_pools));
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
	temp.full_pools.merge(std::move(temp.empty_pools));
	auto const &h = temp.grow();
	if (temp.empty_pools.size() != 1) {
		SYNAFIS_FAILURE("The allocator did not add exactly one empty pool.");
	}
	SYNAFIS_ASSERT(std::addressof(h) == std::addressof(temp.empty_pools.front()));
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
