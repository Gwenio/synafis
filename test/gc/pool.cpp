
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
#include "../../src/gc/impl/idaccess.hpp"
#include "gc.hpp"

#include <algorithm>
#include <stdexcept>
#include <array>
#include <iostream>

using gc::pool;
using gc::free_list;
using gc::blueprint;
using gc::vmem;
using gc::get_id;
using gc::idaccess;

using gc_test::simple;
using gc_test::simple_ptr;

namespace {

blueprint const simple_cfg{
	get_id<simple>(), std::max(idaccess::unit_size<simple>(), blueprint::min_unit())};

blueprint const simple_ptr_cfg{
	get_id<simple_ptr>(), std::max(idaccess::unit_size<simple_ptr>(), blueprint::min_unit())};

}

//!	\cond impl_details

using t = typename unit_test::tester<pool>;
using utc = typename unit_test::collector;

void t::invariants(pool const &obj) noexcept
{
	using node = free_list::node;
	SYNAFIS_ASSERT(obj.free.space <= obj.store.max());
	SYNAFIS_ASSERT((obj.store.front <= obj.free.head && obj.free.head < obj.store.back) ||
				   obj.free.space == 0);
	SYNAFIS_ASSERT(obj.gray.sentinel <= obj.gray.current);
	SYNAFIS_ASSERT(obj.gray.current - obj.gray.sentinel <= obj.used());
	std::size_t count{0};
	bool flag{true};
	for (node *cur = obj.free.head; cur != nullptr; cur = cur->next) {
		count++;
		if (count > obj.free.space) {
			SYNAFIS_FAILURE(
				"The number of nodes in the free list was greater than the free space count.");
			return;
		}
		std::size_t const offset{obj.store.get_slot(cur)};
		if (offset < obj.store.max()) {
			flag |= !obj.initialized.test(offset);
		} else {
			std::cout << offset << " "
					  << (static_cast<std::byte *>(obj.store.front) -
							 static_cast<std::byte *>(static_cast<void *>(cur))) /
							 obj.store.unit
					  << std::endl;
			SYNAFIS_FAILURE("A free list node pointed to an address outside the pool.");
			return;
		}
	}
	if (count < obj.free.space) {
		SYNAFIS_FAILURE("The number of nodes in the free list was less than the free space count.");
	}
	if (!flag) { SYNAFIS_FAILURE("The allocated bit of a slot in the free list was set."); }
}

void t::creation(utc &)
{
	//!	TEST Test pool::handle constructor that creates a pool.
	pool temp{get_id<simple>(), simple_cfg};
	SYNAFIS_ASSERT(temp.region);
	SYNAFIS_ASSERT(tester<vmem>::is_allocated(temp.region));
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.store.max());
	invariants(temp);
}

void t::destruction(utc &)
{
	void *addr{nullptr};
	{
		pool temp{get_id<simple>(), simple_cfg};
		SYNAFIS_ASSERT(temp.region);
		invariants(temp);
		addr = temp.region.begin();
	}
	//!	TEST Test if a pool's region is deallocated on destruction of the pool.
	SYNAFIS_ASSERT(tester<vmem>::is_free(addr, vmem::page_size));
}

void t::allocation(utc &)
{
	pool temp{get_id<simple>(), simple_cfg};
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.store.max());
	std::array<void *, 16> store;
	for (auto &x : store) {
		x = temp.allocate();
	}
	SYNAFIS_ASSERT(temp.used() == store.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.store.max());
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

void t::sweeping(utc &)
{
	pool temp{get_id<simple>(), simple_cfg};
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.store.max());
	invariants(temp);
	std::array<void *, 8> store1;
	std::array<void *, 16> store2;
	for (auto &x : store1) {
		x = temp.allocate();
	}
	for (auto &x : store2) {
		x = temp.allocate();
	}
	SYNAFIS_ASSERT(temp.used() == store1.size() + store2.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.store.max());
	invariants(temp);
	for (auto x : store1) {
		temp.mark(x);
	}
	//!	TEST marking did not cause deallocation
	SYNAFIS_ASSERT(temp.used() == store1.size() + store2.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.store.max());
	invariants(temp);
	//!	TEST sweeping does deallocate the correct slots
	temp.sweep();
	SYNAFIS_ASSERT(temp.used() == store1.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.store.max());
	invariants(temp);
	//!	TEST sweeping again without marking deallocates everything
	temp.sweep();
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.store.max());
	invariants(temp);
}

void t::discarding(utc &)
{
	pool temp{get_id<simple>(), simple_cfg};
	SYNAFIS_ASSERT(temp.used() == 0);
	SYNAFIS_ASSERT(temp.available() == temp.store.max());
	std::array<void *, 8> store1;
	std::array<void *, 16> store2;
	for (auto &x : store1) {
		x = temp.allocate();
	}
	for (auto &x : store2) {
		x = temp.allocate();
	}
	SYNAFIS_ASSERT(temp.used() == store1.size() + store2.size());
	SYNAFIS_ASSERT(temp.available() + temp.used() == temp.store.max());
	for (auto x : store1) {
		temp.discarded(x);
	}
	//!	TEST discarding slots returns them to the pool
	SYNAFIS_ASSERT(temp.used() == store2.size());
	invariants(temp);
}

void t::traversing(utc &)
{
	pool temp1{get_id<simple>(), simple_cfg};
	pool temp2{get_id<simple_ptr>(), simple_ptr_cfg};
	invariants(temp2);
	std::array<simple *, 32> store1;
	std::array<simple_ptr *, 16> store2;
	for (auto &x : store1) {
		x = static_cast<simple *>(temp1.allocate());
	}
	for (std::size_t count = 0; count < store2.size(); count++) {
		simple_ptr *ref{static_cast<simple_ptr *>(temp2.allocate())};
		ref->data = store1[count * 2];
		store2[count] = ref;
	}
	invariants(temp2);
	for (simple_ptr *x : store2) {
		temp2.mark(x);
	}
	invariants(temp2);
	auto const cb = [](void *arg, void *ptr) noexcept->void
	{
		pool *p{static_cast<pool *>(arg)};
		SYNAFIS_ASSERT(p->from(ptr));
		p->mark(ptr);
	};
	temp2.traverse(std::addressof(temp1), cb);
	invariants(temp1);
	invariants(temp2);
	//!	TEST traversal marks all the pointed to slots
	temp1.sweep();
	temp2.sweep();
	SYNAFIS_ASSERT(temp1.used() == temp2.used());
	invariants(temp1);
	invariants(temp2);
}

//!	\endcond

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s() noexcept { return gc_test::pool_suite; }

c traversing{"traversing", s(), pass, &t::traversing};

c discarding{"discarding", s(), pass, &t::discarding};

c sweeping{"sweeping", s(), pass, &t::sweeping};

c allocation{"allocation", s(), pass, &t::allocation};

c destruction{"destruction", s(), pass, &t::destruction};

c creation{"creation", s(), pass, &t::creation};

}
