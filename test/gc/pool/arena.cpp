
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

/**	\file test/gc/pool/arena.hpp
 *	\brief Defines test cases for gc::arena.
 */

#include "arena.hpp"

#include <algorithm>
#include <array>
#include <random>

using gc::arena;

namespace {

using buffer = std::array<std::size_t, 4>;

constexpr std::size_t buffer_size() noexcept { return buffer{0}.size(); }

constexpr std::size_t footprint() noexcept { return buffer_size() * sizeof(std::size_t); }

}

//!	\cond impl_details

using t = typename unit_test::tester<arena>;
using utc = typename unit_test::collector;

void t::creation(utc &)
{
	//!	TEST that gc::arena initializes correctly.
	buffer region{};
	arena temp{buffer_size(), sizeof(std::size_t), region.data(), footprint()};
	SYNAFIS_ASSERT(temp.front == temp.begin());
	SYNAFIS_ASSERT(temp.front == temp.cbegin());
	SYNAFIS_ASSERT(temp.back == temp.end());
	SYNAFIS_ASSERT(temp.back == temp.cend());
	SYNAFIS_ASSERT(temp.front < temp.back);
	SYNAFIS_ASSERT(temp.unit == sizeof(std::size_t));
	SYNAFIS_ASSERT(temp.capacity == region.size());
	//!	TEST the gc::arena copy constructor.
	arena copy{temp};
	SYNAFIS_ASSERT(temp.front == copy.front);
	SYNAFIS_ASSERT(temp.back == copy.back);
	SYNAFIS_ASSERT(temp.unit == copy.unit);
	SYNAFIS_ASSERT(temp.capacity == copy.capacity);
}

void t::ownership(utc &)
{
	std::array<buffer, 3> buffers{{{0, 0, 0, 0}, {1, 1, 1, 1}, {2, 2, 2, 2}}};
	arena x{buffer_size(), sizeof(std::size_t), buffers[0].data(), footprint()};
	arena y{buffer_size(), sizeof(std::size_t), buffers[1].data(), footprint()};
	arena z{buffer_size(), sizeof(std::size_t), buffers[2].data(), footprint()};
	//!	TEST that arenas do not say nullptr is from them.
	SYNAFIS_ASSERT(!x.from(nullptr));
	SYNAFIS_ASSERT(!y.from(nullptr));
	SYNAFIS_ASSERT(!z.from(nullptr));
	std::array<std::size_t *, buffer_size() * 3> store;
	{
		auto cur = store.begin();
		for (auto &b : buffers) {
			for (auto &i : b) {
				*cur = std::addressof(i);
				cur++;
			}
		}
		std::random_device rd{};
		std::mt19937_64 g{rd()};
		std::shuffle(store.begin(), store.end(), g);
	}
	//!	TEST that arenas only claim pointers that are in their memory region
	for (std::size_t *const i : store) {
		bool in_arena{true};
		bool not_arena{false};
		switch (*i) {
		case 0:
			in_arena &= x.from(i);
			not_arena |= (y.from(i) || z.from(i));
			break;
		case 1:
			in_arena &= y.from(i);
			not_arena |= (x.from(i) || z.from(i));
			break;
		case 2:
			in_arena &= z.from(i);
			not_arena |= (y.from(i) || x.from(i));
			break;
		default:
			SYNAFIS_FAILURE("Invalid arena number.");
			return;
		}
		if (!in_arena) {
			SYNAFIS_FAILURE("At least one pointer was not identified as from the correct arena.");
			return;
		} else if (not_arena) {
			SYNAFIS_FAILURE("At least one pointer was identified as from an incorrect arena.");
			return;
		}
	}
}

void t::offsets(utc &)
{
	buffer region{};
	arena temp{buffer_size(), sizeof(std::size_t), region.data(), footprint()};
	for (std::size_t count = 0; count < region.size(); count++) {
		region[count] = count;
	}
	std::random_device rd{};
	std::mt19937_64 g{rd()};
	std::shuffle(region.begin(), region.end(), g);
	//!	TEST that gc::arena can determine the slot of a pointer.
	for (std::size_t count = 0; count < region.size(); count++) {
		std::size_t const slot{region[count]};
		std::size_t *const ptr{std::addressof(region[slot])};
		SYNAFIS_ASSERT(temp.get_slot(ptr) == slot);
	}
}

void t::accessing(utc &)
{
	buffer region{0};
	arena temp{buffer_size(), sizeof(std::size_t), region.data(), footprint()};
	//!	TEST that gc::arena can get a pointer to a slot at an offset.
	for (std::size_t count = 0; count < region.size(); count++) {
		SYNAFIS_ASSERT(temp[count] == std::addressof(region[count]));
	}
}

//!	\endcond

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s() noexcept { return gc_test::arena_suite; }

c accessing{"accessing", s(), pass, &t::accessing};

c offsets{"offsets", s(), pass, &t::offsets};

c ownership{"ownership", s(), pass, &t::ownership};

c creation{"creation", s(), pass, &t::creation};

}
