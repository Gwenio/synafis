
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

/**	\file test/gc/allocator.cpp
 *	\brief Defines test cases for gc::allocator.
 */

#include "allocator.hpp"
#include "../../src/gc/idaccess.hpp"

#include <algorithm>
#include <stdexcept>
#include <array>

namespace {

/**	\class simple
 *	\brief A simple type for allocator testing.
 */
class simple {
public:
	simple() = default;
	~simple() = default;
	std::uintptr_t data;
};

static gc::identity const id(static_cast<simple *>(nullptr));

static std::size_t const simple_unit{std::max(gc::idaccess::unit_size<simple>(), gc::pool::min_unit)};

static auto const simple_flags = gc::traits::get_flags<simple>();

using t = unit_test::tester<gc::allocator>;

}

namespace unit_test {

//!	\cond impl_details

void t::creation(collector &) {
	allocator temp{id, simple_unit, simple_flags};
	SYNAFIS_ASSERT(!temp.pools.empty());
	SYNAFIS_ASSERT(temp.pools.front());
}

void t::growth(collector &) {
	allocator temp{id, simple_unit, simple_flags};
	SYNAFIS_ASSERT(!temp.pools.empty());
	auto &h = temp.grow();
	SYNAFIS_ASSERT(std::addressof(h) == std::addressof(temp.pools.front()));
	std::size_t x{0};
	for (auto &y : temp.pools) {
		if (y) {
			x++;
		} else {
			SYNAFIS_FAILURE("A pool was not allocated.");
			return;
		}
	}
	if (x < 2) {
		SYNAFIS_FAILURE("The allocator contained too few pools.");
	} else if (x > 2) {
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

static unit_test::suite s{"allocator", unit_test::gc_suite};

static c growth{"growth", s, pass, &t::growth};

static c creation{"creation", s, pass, &t::creation};

}
