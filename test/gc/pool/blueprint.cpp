
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

/**	\file test/gc/pool/blueprint.hpp
 *	\brief Defines test cases for gc::blueprint.
 */

#include "blueprint.hpp"

#include "../../../src/gc/impl/pool/blueprint.hpp"
#include "suites.hpp"

#include "../../../src/gc/impl/vmem.hpp"
#include "../../../src/gc/identity.hpp"
#include "../../../src/gc/impl/idaccess.hpp"
#include "../vmem.hpp"

using gc::vmem;
using gc::blueprint;
using gc::get_id;
using gc::idaccess;
using gc_test::simple;
using gc_test::simple_ptr;

//!	\cond impl_details

using t = typename unit_test::tester<blueprint>;
using utc = typename unit_test::collector;

void t::creation(utc &)
{
	//!	TEST that gc::blueprint initializes correctly.
	blueprint temp{get_id<simple_ptr>(), idaccess::unit_size<simple_ptr>()};
	SYNAFIS_ASSERT(temp.unit >= blueprint::min_unit());
	SYNAFIS_ASSERT(temp.unit % alignof(simple_ptr) == 0);
	SYNAFIS_ASSERT(temp.unit >= sizeof(simple_ptr));
}

void t::prepare_region(utc &)
{
	blueprint temp{get_id<simple_ptr>(), idaccess::unit_size<simple_ptr>()};
	vmem region{temp.prepare_region()};
	//!	TEST that gc::blueprint prepare_region() is correct.
	SYNAFIS_ASSERT(region.size() == temp.total);
	SYNAFIS_ASSERT(tester<vmem>::is_writable(region, temp.maps, temp.header));
	SYNAFIS_ASSERT(tester<vmem>::is_writable(region, temp.begin, temp.length));
}

void t::prepare_gray(utc &)
{
	{
		blueprint temp{get_id<simple_ptr>(), idaccess::unit_size<simple_ptr>()};
		vmem region{temp.prepare_region()};
		void **const gray{temp.prepare_gray(get_id<simple_ptr>(), region)};
		//!	TEST that gc::blueprint prepare_gray() is correct when there is a gray list.
		SYNAFIS_ASSERT(temp.gray > temp.maps);
		SYNAFIS_ASSERT(temp.gray < temp.maps + temp.header);
		SYNAFIS_ASSERT(gray == region[temp.gray]);
	}
	{
		blueprint temp{get_id<simple>(), idaccess::unit_size<simple>()};
		vmem region{temp.prepare_region()};
		void **const gray{temp.prepare_gray(get_id<simple>(), region)};
		//!	TEST that gc::blueprint prepare_gray() is correct when there is no a gray list.
		SYNAFIS_ASSERT(temp.gray == 0);
		SYNAFIS_ASSERT(gray == nullptr);
	}
}

//!	\endcond

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s() noexcept { return gc_test::blueprint_suite; }

c prepare_gray{"prepare_gray", s(), pass, &t::prepare_gray};

c prepare_region{"prepare_region", s(), pass, &t::prepare_region};

c creation{"creation", s(), pass, &t::creation};

}
