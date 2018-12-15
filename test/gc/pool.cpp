
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

/**	\file test/gc/pool.cpp
 *	\brief Defines test cases for gc::pool.
 */

#include "pool.hpp"
#include "vmem.hpp"
#include "../../src/gc/idaccess.hpp"

#include <algorithm>
#include <stdexcept>

namespace {

/**	\class simple
 *	\brief A simple type for pool testing.
 */
class simple {
public:
	simple() = default;
	~simple() = default;
	std::uintptr_t data;
};

static gc::identity const id(static_cast<simple *>(nullptr));

static std::size_t const simple_unit{std::max(gc::idaccess::unit_size<simple>(), gc::pool::min_unit)};

static std::size_t const simple_cap{gc::pool::select_capacity(simple_unit)};

}

namespace unit_test {

void tester<gc::pool>::capacity_selection(collector &) {
	{
		// Test for normal cases.
		auto const cap{pool::select_capacity(pool::min_unit)};
		SYNAFIS_ASSERT(config::min_pool <= cap);
		SYNAFIS_ASSERT(cap * pool::min_unit <= config::max_pool * gc::vmem::page_size);
	}
	{
		// Test for large objects.
		std::size_t const unit{(config::max_pool * gc::vmem::page_size / config::min_pool) + pool::min_unit};
		auto const cap{pool::select_capacity(unit)};
		SYNAFIS_ASSERT(config::min_pool <= cap);
		SYNAFIS_ASSERT(cap * unit >= config::max_pool * gc::vmem::page_size);
	}
}

void tester<gc::pool>::null_handle(collector &) {
	{
		handle temp{};
		SYNAFIS_ASSERT(temp.ptr == nullptr);
	}
	{
		handle temp{nullptr};
		SYNAFIS_ASSERT(temp.ptr == nullptr);
	}
}

void tester<gc::pool>::creation(collector &) {
	{
		handle temp{};
		SYNAFIS_ASSERT(temp.ptr == nullptr);
	}
	{
		handle temp{nullptr};
		SYNAFIS_ASSERT(temp.ptr == nullptr);
	}
}

void tester<gc::pool>::destruction(collector &) {
	void *addr{nullptr};
	{
		handle temp{id, simple_cap, simple_unit};
		SYNAFIS_ASSERT(temp.ptr != nullptr);
		addr = temp.ptr;
		SYNAFIS_ASSERT(tester<gc::vmem>::is_allocated(temp.ptr->region));
	}
	SYNAFIS_ASSERT(tester<gc::vmem>::is_free(addr, gc::vmem::page_size));
}

}

namespace {

using t = unit_test::tester<gc::pool>;
using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

static unit_test::suite s{"pool", unit_test::gc_suite};

static c destruction{"null handle", s, pass, &t::destruction};

static c creation{"creation", s, pass, &t::creation};

static c null_handle{"null handle", s, pass, &t::null_handle};

static c capacity{"capacity", s, pass, &t::capacity_selection};

}
