
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
#include <array>

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
	handle temp{id, simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.ptr != nullptr);
	SYNAFIS_ASSERT(tester<gc::vmem>::is_allocated(temp.ptr->region));
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
}

void tester<gc::pool>::destruction(collector &) {
	void *addr{nullptr};
	{
		handle temp{id, simple_cap, simple_unit};
		SYNAFIS_ASSERT(temp.ptr != nullptr);
		addr = temp.ptr;
	}
	SYNAFIS_ASSERT(tester<gc::vmem>::is_free(addr, gc::vmem::page_size));
	handle temp{id, simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.ptr != nullptr);
	addr = temp.ptr;
	temp = nullptr;
	SYNAFIS_ASSERT(tester<gc::vmem>::is_free(addr, gc::vmem::page_size));
}

void tester<gc::pool>::boolean(collector &) {
	handle x{id, simple_cap, simple_unit};
	handle y{};
	handle z{};
	SYNAFIS_ASSERT(x);
	SYNAFIS_ASSERT(x != nullptr);
	SYNAFIS_ASSERT(x == x);
	SYNAFIS_ASSERT(!y);
	SYNAFIS_ASSERT(y == nullptr);
	SYNAFIS_ASSERT(y == y);
	SYNAFIS_ASSERT(x != y);
	SYNAFIS_ASSERT(!z);
	SYNAFIS_ASSERT(z == nullptr);
	SYNAFIS_ASSERT(z == z);
	SYNAFIS_ASSERT(z == y);
}

void tester<gc::pool>::moving(collector &) {
	handle x{id, simple_cap, simple_unit};
	handle y{};
	void *addr{nullptr};
	SYNAFIS_ASSERT(x.ptr != nullptr);
	SYNAFIS_ASSERT(tester<gc::vmem>::is_allocated(x.ptr->region));
	SYNAFIS_ASSERT(y.ptr == nullptr);
	y = std::move(x);
	SYNAFIS_ASSERT(y.ptr != nullptr);
	SYNAFIS_ASSERT(tester<gc::vmem>::is_allocated(y.ptr->region));
	SYNAFIS_ASSERT(x.ptr == nullptr);
	y = std::move(y);
	SYNAFIS_ASSERT(y.ptr != nullptr);
	SYNAFIS_ASSERT(tester<gc::vmem>::is_allocated(y.ptr->region));
	SYNAFIS_ASSERT(x.ptr == nullptr);
	addr = y.ptr;
	y = std::move(x);
	SYNAFIS_ASSERT(x.ptr == nullptr);
	SYNAFIS_ASSERT(y.ptr == nullptr);
	SYNAFIS_ASSERT(tester<gc::vmem>::is_free(addr, gc::vmem::page_size));
}

void tester<gc::pool>::allocation(collector &) {
	handle temp{id, simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
	std::array<void *, 16> store;
	for (auto &x : store) {
		x = temp.allocate();
	}
	SYNAFIS_ASSERT(temp.used() == store.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	bool in_pool{true};
	for (auto x : store) {
		in_pool &= temp.from(x);
	}
	SYNAFIS_ASSERT(in_pool);
}

void tester<gc::pool>::sweeping(collector &) {
	handle temp{id, simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
	std::array<void *, 8> store1;
	std::array<void *, 16> store2;
	bool in_pool{true};
	for (auto &x : store1) {
		x = temp.allocate();
		in_pool &= temp.from(x);
	}
	for (auto &x : store2) {
		x = temp.allocate();
		in_pool &= temp.from(x);
	}
	SYNAFIS_ASSERT(in_pool);
	SYNAFIS_ASSERT(temp.used() == store1.size() + store2.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	for (auto x : store1) {
			temp.mark(x);
	}
	SYNAFIS_ASSERT(temp.used() == store1.size() + store2.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	temp.sweep();
	SYNAFIS_ASSERT(temp.used() == store1.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	in_pool = true;
	for (auto x : store1) {
		in_pool &= temp.from(x);
	}
	SYNAFIS_ASSERT(in_pool);
	temp.sweep();
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
}

}

namespace {

using t = unit_test::tester<gc::pool>;
using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

static unit_test::suite s{"pool", unit_test::gc_suite};

static c sweeping{"sweeping", s, pass, &t::sweeping};

static c allocation{"allocation", s, pass, &t::allocation};

static c moving{"moving", s, pass, &t::moving};

static c boolean{"boolean", s, pass, &t::boolean};

static c destruction{"destruction", s, pass, &t::destruction};

static c creation{"creation", s, pass, &t::creation};

static c null_handle{"null handle", s, pass, &t::null_handle};

static c capacity{"capacity", s, pass, &t::capacity_selection};

}
