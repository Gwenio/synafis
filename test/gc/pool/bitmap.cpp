
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

/**	\file test/gc/pool/bitmap.hpp
 *	\brief Defines test cases for gc::bitmap.
 */

#include "bitmap.hpp"

#include "../../../src/gc/impl/pool/bitmap.hpp"
#include "suites.hpp"

#include "../../../src/gc/impl/vmem.hpp"
#include "../../../src/gc/identity.hpp"
#include "../../../src/gc/impl/idaccess.hpp"

#include <algorithm>
#include <array>
#include <random>

using gc::bitmap;
using gc::get_id;
using gc::idaccess;
using gc_test::simple;
using gc_test::simple_ptr;

namespace {

constexpr std::size_t capacity{bitmap::bits() * 4};

}

//!	\cond impl_details

using buffer = typename std::array<std::bitset<bitmap::bits()>, 4>;

using control = typename std::array<bool, capacity>;

using t = typename unit_test::tester<bitmap>;
using utc = typename unit_test::collector;

void t::creation(utc &)
{
	//!	TEST gc::bitmap default constructor.
	bitmap temp1{};
	SYNAFIS_ASSERT(temp1.data == nullptr);
	//!	TEST gc::bitmap constructor.
	buffer b{};
	bitmap temp2{b.data()};
	SYNAFIS_ASSERT(temp2.data == b.data());
	//!	TEST gc::bitmap copy constructor.
	bitmap temp3{temp1};
	SYNAFIS_ASSERT(temp3.data == nullptr);
	bitmap temp4{temp2};
	SYNAFIS_ASSERT(temp4.data == temp2.data);
	bitmap temp5{bitmap{b.data()}};
	SYNAFIS_ASSERT(temp4.data == b.data());
	//!	TEST gc::bitmap copy assignment.
	temp1 = temp2;
	SYNAFIS_ASSERT(temp1.data == b.data());
	SYNAFIS_ASSERT(temp2.data == b.data());
	temp4 = temp3;
	SYNAFIS_ASSERT(temp3.data == nullptr);
	SYNAFIS_ASSERT(temp4.data == nullptr);
	temp4 = bitmap{b.data()};
	SYNAFIS_ASSERT(temp4.data == b.data());
	temp5 = bitmap{};
	SYNAFIS_ASSERT(temp5.data == nullptr);
}

void t::swapping(utc &)
{
	bitmap temp1{};
	buffer b1{};
	buffer b2{};
	bitmap temp2{b1.data()};
	bitmap temp3{b2.data()};
	SYNAFIS_ASSERT(temp1.data == nullptr);
	//!	TEST swapping bitmaps.
	std::swap(temp1, temp2);
	SYNAFIS_ASSERT(temp2.data == nullptr);
	SYNAFIS_ASSERT(temp1.data == b1.data());
	std::swap(temp1, temp3);
	SYNAFIS_ASSERT(temp1.data == b2.data());
	SYNAFIS_ASSERT(temp3.data == b1.data());
}

void t::checking(utc &)
{
	buffer b{};
	bitmap temp{b.data()};
	control c{};
	{
		std::random_device rd{};
		std::mt19937_64 g{rd()};
		std::bernoulli_distribution d(0.5);
		for (bool &x : c) {
			x = d(g);
		}
		auto cur = c.cbegin();
		for (auto &x : b) {
			for (std::size_t count = 0; count < bitmap::bits(); count++) {
				x[count] = *cur;
				cur++;
			}
		}
	}
	//!	TEST bitmap::test().
	auto cur = c.cbegin();
	for (std::size_t count = 0; count < capacity; count++) {
		if (*cur == temp.test(count)) {
			cur++;
		} else {
			SYNAFIS_FAILURE("gc::bitmap::test() did not return the expected value.");
			return;
		}
	}
}

void t::setting(utc &)
{
	buffer b{};
	bitmap temp{b.data()};
	control c{};
	{
		std::random_device rd{};
		std::mt19937_64 g{rd()};
		std::bernoulli_distribution d(0.5);
		for (bool &x : c) {
			x = d(g);
		}
	}
	for (auto &x : b) {
		x.reset();
	}
	//!	TEST bitmap::set().
	for (std::size_t count = 0; count < capacity; count++) {
		if (c[count]) { temp.set(count); }
	}
	auto cur = c.cbegin();
	for (auto &x : b) {
		for (std::size_t count = 0; count < bitmap::bits(); count++) {
			if (x[count] && !*cur) {
				SYNAFIS_FAILURE("gc::bitmap::set() set an incorrect bit.");
				return;
			} else if (!x[count] && *cur) {
				SYNAFIS_FAILURE("gc::bitmap::set() did not set a bit it should have.");
				return;
			} else {
				cur++;
			}
		}
	}
}

void t::resetting(utc &)
{
	buffer b{};
	bitmap temp{b.data()};
	control c{};
	{
		std::random_device rd{};
		std::mt19937_64 g{rd()};
		std::bernoulli_distribution d(0.5);
		for (bool &x : c) {
			x = d(g);
		}
	}
	for (auto &x : b) {
		x.set();
	}
	//!	TEST bitmap::reset().
	for (std::size_t count = 0; count < capacity; count++) {
		if (!c[count]) { temp.reset(count); }
	}
	auto cur = c.cbegin();
	for (auto &x : b) {
		for (std::size_t count = 0; count < bitmap::bits(); count++) {
			if (x[count] && !*cur) {
				SYNAFIS_FAILURE("gc::bitmap::reset() did not reset a bit it should have.");
				return;
			} else if (!x[count] && *cur) {
				SYNAFIS_FAILURE("gc::bitmap::reset() reset an incorrect bit.");
				return;
			} else {
				cur++;
			}
		}
	}
}

//!	\endcond

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s() noexcept { return gc_test::bitmap_suite; }

c resetting{"resetting", s(), pass, &t::resetting};

c setting{"setting", s(), pass, &t::setting};

c checking{"checking", s(), pass, &t::checking};

c swapping{"swapping", s(), pass, &t::swapping};

c creation{"creation", s(), pass, &t::creation};

}
