
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

/**	\file test/gc/pool.cpp
 *	\brief Defines test cases for gc::pool.
 */

#include "pool.hpp"
#include "vmem.hpp"
#include "../../src/gc/idaccess.hpp"
#include "gc.hpp"

#include <algorithm>
#include <stdexcept>
#include <array>

namespace {

using namespace gc;

using simple = unit_test::gc::simple;
using simple_ptr = unit_test::gc::simple_ptr;

std::size_t const simple_unit{std::max(gc::idaccess::unit_size<simple>(), gc::pool::min_unit)};

std::size_t const simple_ptr_unit{
	std::max(gc::idaccess::unit_size<simple_ptr>(), gc::pool::min_unit)};

std::size_t const simple_cap{pool::select_capacity(simple_unit)};

std::size_t const simple_ptr_cap{pool::select_capacity(simple_ptr_unit)};

using t = unit_test::tester<pool>;

}

namespace unit_test {

//!	\cond impl_details

void t::invariants(pool const &obj) noexcept
{
	using node = pool::node;
	SYNAFIS_ASSERT(obj.space <= obj.capacity);
	SYNAFIS_ASSERT((obj.slots <= obj.free && obj.free < obj.end) || obj.space == 0);
	SYNAFIS_ASSERT(obj.sentinel <= obj.gray);
	SYNAFIS_ASSERT(obj.gray - obj.sentinel <= obj.capacity - obj.space);
	std::size_t count{0};
	bool flag{true};
	for (node const *cur = obj.free; cur != nullptr; cur = cur->next) {
		count++;
		std::size_t bit, group;
		std::tie(group, bit) = obj.bit_locate(const_cast<node *>(cur));
		flag |= !obj.bitmap[group].test(bit);
	}
	if (count < obj.space) {
		SYNAFIS_FAILURE("The number of nodes in the free list was less than the free space count.");
	} else if (count > obj.space) {
		SYNAFIS_FAILURE(
			"The number of nodes in the free list was greater than the free space count.");
	}
	if (!flag) { SYNAFIS_FAILURE("The allocated bit of a slot in the free list was set."); }
}

void t::capacity_selection(collector &)
{
	{
		//!	TEST Test capacity selection for small objects.
		auto const cap{pool::select_capacity(pool::min_unit)};
		SYNAFIS_ASSERT(config::min_pool <= cap);
		SYNAFIS_ASSERT(cap * pool::min_unit <= config::max_pool * vmem::page_size);
	}
	{
		//!	TEST Test capacity selection for very large objects.
		std::size_t const unit{
			(config::max_pool * vmem::page_size / config::min_pool) + pool::min_unit};
		auto const cap{pool::select_capacity(unit)};
		SYNAFIS_ASSERT(config::min_pool <= cap);
		SYNAFIS_ASSERT(cap * unit >= config::max_pool * vmem::page_size);
	}
}

void t::null_handle(collector &)
{
	{
		//!	TEST Test default pool::handle constructor.
		handle temp{};
		SYNAFIS_ASSERT(temp.ptr == nullptr);
	}
	{
		//!	TEST Test pool::handle constructor with a nullptr.
		handle temp{nullptr};
		SYNAFIS_ASSERT(temp.ptr == nullptr);
	}
}

void t::creation(collector &)
{
	//!	TEST Test pool::handle constructor that creates a pool.
	handle temp{get_id<simple>(), simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.ptr != nullptr);
	SYNAFIS_ASSERT(tester<vmem>::is_allocated(temp.ptr->region));
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
	invariants(*temp);
}

void t::destruction(collector &)
{
	void *addr{nullptr};
	{
		handle temp{get_id<simple>(), simple_cap, simple_unit};
		SYNAFIS_ASSERT(temp.ptr != nullptr);
		invariants(*temp);
		addr = temp.ptr;
	}
	//!	TEST Test if the memory for a pool is deallocated on handle destruction.
	SYNAFIS_ASSERT(tester<vmem>::is_free(addr, vmem::page_size));
	handle temp{get_id<simple>(), simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.ptr != nullptr);
	invariants(*temp);
	addr = temp.ptr;
	//!	TEST Test if the memory for a pool is deallocated on when setting a handle to nullptr.
	temp = nullptr;
	SYNAFIS_ASSERT(tester<vmem>::is_free(addr, vmem::page_size));
}

void t::boolean(collector &)
{
	handle x{get_id<simple>(), simple_cap, simple_unit};
	handle y{};
	handle z{};
	//!	TEST handle objects correctly convert to true when owning a pool.
	SYNAFIS_ASSERT(x);
	//!	TEST handle objects are not equal to nullptr when owning a pool.
	SYNAFIS_ASSERT(x != nullptr);
	//!	TEST handle objects are equal to themselves.
	SYNAFIS_ASSERT(x == x);
	//!	TEST handle objects operator! is false when not owning a pool.
	SYNAFIS_ASSERT(!y);
	//!	TEST handle objects are equal to nullptr when not owning a pool.
	SYNAFIS_ASSERT(y == nullptr);
	SYNAFIS_ASSERT(y == y);
	SYNAFIS_ASSERT(x != y);
	SYNAFIS_ASSERT(!z);
	SYNAFIS_ASSERT(z == nullptr);
	SYNAFIS_ASSERT(z == z);
	//!	TEST handle objects with no pool are equal.
	SYNAFIS_ASSERT(z == y);
}

void t::moving(collector &)
{
	handle x{get_id<simple>(), simple_cap, simple_unit};
	handle y{};
	void *addr{nullptr};
	//!	TEST moving a pool between handles.
	SYNAFIS_ASSERT(x.ptr != nullptr);
	SYNAFIS_ASSERT(tester<vmem>::is_allocated(x.ptr->region));
	SYNAFIS_ASSERT(y.ptr == nullptr);
	y = std::move(x);
	SYNAFIS_ASSERT(y.ptr != nullptr);
	SYNAFIS_ASSERT(tester<vmem>::is_allocated(y.ptr->region));
	SYNAFIS_ASSERT(x.ptr == nullptr);
	y = std::move(y);
	SYNAFIS_ASSERT(y.ptr != nullptr);
	SYNAFIS_ASSERT(tester<vmem>::is_allocated(y.ptr->region));
	SYNAFIS_ASSERT(x.ptr == nullptr);
	addr = y.ptr;
	y = std::move(x);
	SYNAFIS_ASSERT(x.ptr == nullptr);
	SYNAFIS_ASSERT(y.ptr == nullptr);
	SYNAFIS_ASSERT(tester<vmem>::is_free(addr, vmem::page_size));
}

void t::allocation(collector &)
{
	handle temp{get_id<simple>(), simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
	std::array<void *, 16> store;
	for (auto &x : store) {
		x = temp.allocate();
	}
	SYNAFIS_ASSERT(temp.used() == store.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	auto const stop = store.end() - 1;
	for (auto cur = store.begin(); cur < stop; cur++) {
		for (auto comp = cur + 1; comp < store.end(); comp++) {
			if (*cur == *comp) {
				SYNAFIS_FAILURE("An address was allocated more than once without being freed.");
				return;
			}
		}
	}
}

void t::ownership(collector &)
{
	handle x{get_id<simple>(), simple_cap, simple_unit};
	handle y{get_id<simple>(), simple_cap, simple_unit};
	handle z{get_id<simple>(), simple_cap, simple_unit};
	std::array<simple *, 32> store;
	for (std::size_t i = 0; i < store.size(); i++) {
		if (i % 2 == 0) {
			store[i] = reinterpret_cast<simple *>(x.allocate());
			store[i]->data = 0;
		} else if (i % 3 == 0) {
			store[i] = reinterpret_cast<simple *>(y.allocate());
			store[i]->data = 1;
		} else {
			store[i] = reinterpret_cast<simple *>(z.allocate());
			store[i]->data = 2;
		}
	}
	//!	TEST that pools do not say nullptr is from them.
	SYNAFIS_ASSERT(!x.from(nullptr));
	SYNAFIS_ASSERT(!y.from(nullptr));
	SYNAFIS_ASSERT(!z.from(nullptr));
	//!	TEST that pools claim pointers they allocated, and not those from others
	bool in_pool{true};
	bool not_pool{false};
	for (auto i : store) {
		switch (i->data) {
		case 0:
			in_pool &= x.from(i);
			not_pool |= (y.from(i) || z.from(i));
			break;
		case 1:
			in_pool &= y.from(i);
			not_pool |= (x.from(i) || z.from(i));
			break;
		case 2:
			in_pool &= z.from(i);
			not_pool |= (y.from(i) || x.from(i));
			break;
		default:
			SYNAFIS_FAILURE("Invalid pool number.");
			return;
		}
	}
	if (!in_pool) {
		SYNAFIS_FAILURE("At least one pointer was not identified as from the correct pool.");
	}
	if (not_pool) {
		SYNAFIS_FAILURE("At least one pointer was identified as from an incorrect pool.");
	}
}

void t::sweeping(collector &)
{
	handle temp{get_id<simple>(), simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
	invariants(*temp);
	std::array<void *, 8> store1;
	std::array<void *, 16> store2;
	for (auto &x : store1) {
		x = temp.allocate();
	}
	for (auto &x : store2) {
		x = temp.allocate();
	}
	SYNAFIS_ASSERT(temp.used() == store1.size() + store2.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	invariants(*temp);
	for (auto x : store1) {
		temp.mark(x);
	}
	//!	TEST marking did not cause deallocation
	SYNAFIS_ASSERT(temp.used() == store1.size() + store2.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	invariants(*temp);
	//!	TEST sweeping does deallocate the correct slots
	temp.sweep();
	SYNAFIS_ASSERT(temp.used() == store1.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	invariants(*temp);
	//!	TEST sweeping again without marking deallocates everything
	temp.sweep();
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
	invariants(*temp);
}

void t::discarding(collector &)
{
	handle temp{get_id<simple>(), simple_cap, simple_unit};
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.ptr->capacity);
	std::array<void *, 8> store1;
	std::array<void *, 16> store2;
	for (auto &x : store1) {
		x = temp.allocate();
	}
	for (auto &x : store2) {
		x = temp.allocate();
	}
	SYNAFIS_ASSERT(temp.used() == store1.size() + store2.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.ptr->capacity);
	for (auto x : store1) {
		temp.discarded(x);
	}
	//!	TEST discarding slots returns them to the pool
	SYNAFIS_ASSERT(temp.used() == store2.size());
	invariants(*temp);
}

void t::traversing(collector &)
{
	handle temp1{get_id<simple>(), simple_cap, simple_unit};
	handle temp2{get_id<simple_ptr>(), simple_ptr_cap, simple_ptr_unit};
	std::array<void *, 32> store1;
	std::array<void *, 16> store2;
	for (auto &x : store1) {
		x = temp1.allocate();
	}
	{
		std::size_t count{0};
		for (auto &x : store2) {
			x = temp2.allocate();
			simple_ptr &ref{*static_cast<simple_ptr *>(x)};
			ref.data = static_cast<simple *>(store1[count * 2]);
			count++;
		}
	}
	for (auto x : store2) {
		temp2.mark(x);
	}
	auto const cb = [](void *arg, void *ptr) noexcept->void
	{
		static_cast<pool *>(arg)->mark(ptr);
	};
	(*temp2).traverse(std::addressof(*temp1), cb);
	//!	TEST traversal marks all the pointed to slots
	temp1.sweep();
	temp2.sweep();
	SYNAFIS_ASSERT(temp1.used() == temp2.used());
	invariants(*temp1);
	invariants(*temp2);
}

//!	\endcond

}

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s{unit_test::gc::pool_suite};

static c traversing{"traversing", s, pass, &t::traversing};

static c discarding{"discarding", s, pass, &t::discarding};

static c sweeping{"sweeping", s, pass, &t::sweeping};

static c ownership{"ownership", s, pass, &t::ownership};

static c allocation{"allocation", s, pass, &t::allocation};

static c moving{"moving", s, pass, &t::moving};

static c boolean{"boolean", s, pass, &t::boolean};

static c destruction{"destruction", s, pass, &t::destruction};

static c creation{"creation", s, pass, &t::creation};

static c null_handle{"null handle", s, pass, &t::null_handle};

static c capacity{"capacity", s, pass, &t::capacity_selection};

}
