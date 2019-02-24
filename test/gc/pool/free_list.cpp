
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

/**	\file test/gc/pool/free_list.hpp
 *	\brief Defines test cases for gc::free_list.
 */

#include "free_list.hpp"
#include "../../../src/gc/impl/pool/arena.hpp"

#include <algorithm>
#include <array>
#include <random>

using gc::free_list;
using gc::arena;

namespace {

using buffer = std::array<unit_test::tester<free_list>::node, 4>;

constexpr std::size_t buffer_size() noexcept { return buffer{0}.size(); }

constexpr std::size_t footprint() noexcept { return buffer_size() * sizeof(std::size_t); }

}

//!	\cond impl_details

using t = typename unit_test::tester<free_list>;
using utc = typename unit_test::collector;

bool t::invariants(free_list const &obj, arena const &area) noexcept
{
	if (obj.space > area.max()) {
		SYNAFIS_FAILURE("The free space in the list exceeds the capacity for the arena.");
		return false;
	} else if (obj.space != obj.available()) {
		SYNAFIS_FAILURE("The free_list's space and available() do not match.");
		return false;
	} else if ((obj.space == 0 && !obj.full()) || (obj.space != 0 && obj.full())) {
		SYNAFIS_FAILURE("The free_list is not correctly identifying itself as full or not.");
		return false;
	}
	std::size_t count{0};
	node *current{obj.head};
	while (current != nullptr) {
		if (count >= obj.space) {
			SYNAFIS_FAILURE("The number of nodes in the free_list exceeded the expected count.");
			return false;
		} else if (!area.from(current)) {
			SYNAFIS_FAILURE("A node in the free_list was outside the expected address range.");
			return false;
		} else {
			count++;
			current = current->next;
		}
	}
	if (count != obj.space) {
		SYNAFIS_FAILURE("The number of nodes counted does not match the free_list's space.");
		return false;
	} else {
		return true;
	}
}

void t::creation(utc &)
{
	buffer region{};
	arena area{buffer_size(), sizeof(node), region.data(), footprint()};
	//!	TEST that gc::free_list initializes correctly.
	free_list temp{area};
	SYNAFIS_ASSERT(temp.space == area.max());
	invariants(temp, area);
}

void t::pushing(utc &)
{
	buffer region{};
	arena area{buffer_size(), sizeof(node), region.data(), footprint()};
	free_list temp{area};
	std::array<void *, region.size()> order{};
	for (std::size_t count = 0; count < region.size(); count++) {
		order[count] = temp.pop();
	}
	{
		std::random_device rd{};
		std::mt19937_64 g{rd()};
		std::shuffle(order.begin(), order.end(), g);
	}
	//!	TEST that gc::free_list pushes new values correctly.
	for (std::size_t count = 0; count < region.size(); count++) {
		node *const previous{temp.head};
		temp.push(order[count]);
		SYNAFIS_ASSERT(temp.available() == 1 + count);
		SYNAFIS_ASSERT(temp.head == order[count]);
		SYNAFIS_ASSERT(previous == temp.head->next);
		if (!invariants(temp, area)) { return; }
	}
	//!	TEST that pushed values are popped in the reverse order.
	for (auto cur = order.rbegin(); cur != order.rend(); cur++) {
		SYNAFIS_ASSERT(*cur == temp.pop());
	}
	SYNAFIS_ASSERT(temp.head == nullptr);
}

void t::popping(utc &)
{
	buffer region{};
	arena area{buffer_size(), sizeof(node), region.data(), footprint()};
	free_list temp{area};
	//!	TEST that gc::free_list pops values correctly.
	for (std::size_t count = 0; count < region.size(); count++) {
		node *const previous{temp.head};
		void *const popped{temp.pop()};
		SYNAFIS_ASSERT(previous == popped);
		SYNAFIS_ASSERT(previous->next == temp.head);
		SYNAFIS_ASSERT(temp.space == temp.available());
		SYNAFIS_ASSERT(temp.available() == area.max() - 1 - count);
		if (!invariants(temp, area)) { return; }
	}
	SYNAFIS_ASSERT(temp.head == nullptr);
}

//!	\endcond

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s() noexcept { return gc_test::free_list_suite; }

c pushing{"pushing", s(), pass, &t::pushing};

c popping{"popping", s(), pass, &t::popping};

c creation{"creation", s(), pass, &t::creation};

}
