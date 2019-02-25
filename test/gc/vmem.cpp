
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

/**	\file test/gc/vmem.cpp
 *	\brief Defines test cases for gc::vmem.
 */

#include "vmem.hpp"
#include "gc.hpp"
#include "../../src/gc/impl/vmem.hpp"

using gc::vmem;
using gc::add_offset;

//!	\cond impl_details

using t = typename unit_test::tester<vmem>;
using utc = typename unit_test::collector;

bool t::invariants(vmem const &obj) noexcept
{
	return !obj.ptr && (obj.len == 0) || obj.ptr && (0 < obj.len) && is_allocated(obj);
}

void t::sane_page_size(utc &)
{
	SYNAFIS_ASSERT(0 < vmem::page_size());
	SYNAFIS_ASSERT(alignof(std::max_align_t) < vmem::page_size());
	SYNAFIS_ASSERT(vmem::page_size() % alignof(std::max_align_t) == 0);
}

void t::def_init(utc &)
{
	vmem temp{};
	SYNAFIS_ASSERT(temp.ptr == nullptr);
	SYNAFIS_ASSERT(temp.len == 0);
}

void t::reg_init(utc &)
{
	{
		vmem temp{vmem::page_size(), true};
		SYNAFIS_ASSERT(temp.ptr != nullptr);
		SYNAFIS_ASSERT(temp.len == vmem::page_size());
		SYNAFIS_ASSERT(is_allocated(temp));
		SYNAFIS_ASSERT(is_writable(temp, 0, vmem::page_size()));
	}
	{
		vmem temp{vmem::page_size(), false};
		SYNAFIS_ASSERT(temp.ptr != nullptr);
		SYNAFIS_ASSERT(temp.len == vmem::page_size());
		SYNAFIS_ASSERT(is_allocated(temp));
		SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size()));
	}
}

void t::move_init(utc &)
{
	vmem temp1{vmem::page_size(), true};
	SYNAFIS_ASSERT(temp1.ptr != nullptr);
	{
		void *const addr{temp1.ptr};
		vmem temp2{std::move(temp1)};
		SYNAFIS_ASSERT(temp1.ptr == nullptr);
		SYNAFIS_ASSERT(temp2.ptr == addr);
		SYNAFIS_ASSERT(invariants(temp1) && invariants(temp2));
	}
	{
		vmem temp2{std::move(temp1)};
		SYNAFIS_ASSERT(temp1.ptr == nullptr);
		SYNAFIS_ASSERT(temp2.ptr == nullptr);
		SYNAFIS_ASSERT(invariants(temp1) && invariants(temp2));
	}
}

void t::destruct(utc &)
{
	void *addr{nullptr};
	{
		vmem temp{vmem::page_size(), true};
		SYNAFIS_ASSERT(is_allocated(temp));
		addr = temp.ptr;
	}
	SYNAFIS_ASSERT(is_free(addr, vmem::page_size()));
}

void t::null_assign(utc &)
{
	vmem temp{vmem::page_size(), true};
	SYNAFIS_ASSERT(temp.ptr != nullptr);
	SYNAFIS_ASSERT(invariants(temp));
	void *addr{temp.ptr};
	temp = nullptr;
	SYNAFIS_ASSERT(temp.ptr == nullptr);
	SYNAFIS_ASSERT(invariants(temp));
	SYNAFIS_ASSERT(is_free(addr, vmem::page_size()));
}

void t::move_assign(utc &)
{
	{
		vmem temp1{vmem::page_size(), true};
		vmem temp2{};
		SYNAFIS_ASSERT(temp1.ptr != nullptr && invariants(temp1));
		SYNAFIS_ASSERT(temp2.ptr == nullptr && invariants(temp2));
		void *addr{temp1.ptr};
		temp1 = std::move(temp2);
		SYNAFIS_ASSERT(temp1.ptr == nullptr && invariants(temp1));
		SYNAFIS_ASSERT(temp2.ptr == nullptr && invariants(temp2));
		SYNAFIS_ASSERT(is_free(addr, vmem::page_size()));
	}
	{
		vmem temp1{};
		vmem temp2{vmem::page_size(), true};
		SYNAFIS_ASSERT(temp1.ptr == nullptr && invariants(temp1));
		SYNAFIS_ASSERT(temp2.ptr != nullptr && invariants(temp2));
		void *addr{temp2.ptr};
		temp1 = std::move(temp2);
		SYNAFIS_ASSERT(temp1.ptr == addr && invariants(temp1));
		SYNAFIS_ASSERT(temp2.ptr == nullptr && invariants(temp2));
	}
	{
		vmem temp1{vmem::page_size(), true};
		vmem temp2{vmem::page_size(), true};
		SYNAFIS_ASSERT(temp1.ptr != nullptr && invariants(temp1));
		SYNAFIS_ASSERT(temp2.ptr != nullptr && invariants(temp2));
		void *addr1{temp1.ptr};
		void *addr2{temp2.ptr};
		temp1 = std::move(temp2);
		SYNAFIS_ASSERT(temp1.ptr == addr2 && invariants(temp1));
		SYNAFIS_ASSERT(temp2.ptr == nullptr && invariants(temp2));
		SYNAFIS_ASSERT(is_free(addr1, vmem::page_size()));
	}
	{
		vmem temp{vmem::page_size(), true};
		SYNAFIS_ASSERT(temp.ptr != nullptr && invariants(temp));
		void *addr{temp.ptr};
		temp = std::move(temp);
		SYNAFIS_ASSERT(temp.ptr == addr && invariants(temp));
	}
}

void t::bool_convert(utc &)
{
	vmem temp1{};
	vmem temp2{vmem::page_size(), true};
	SYNAFIS_ASSERT(!temp1);
	SYNAFIS_ASSERT(temp2);
}

void t::bounds(utc &)
{
	vmem temp1{};
	vmem temp2{vmem::page_size(), true};
	SYNAFIS_ASSERT(temp1.begin() == nullptr);
	SYNAFIS_ASSERT(temp1.end() == nullptr);
	SYNAFIS_ASSERT(temp1.size() == 0);
	SYNAFIS_ASSERT(temp2.begin() == temp2.ptr);
	SYNAFIS_ASSERT(temp2.end() == add_offset(temp2.ptr, temp2.len));
	SYNAFIS_ASSERT(temp2.size() == temp2.len);
}

void t::access(utc &)
{
	vmem temp{vmem::page_size(), true};
	SYNAFIS_ASSERT(temp[0] == temp.ptr);
	SYNAFIS_ASSERT(temp[vmem::page_size() / 2] == add_offset(temp.ptr, temp.len / 2));
	SYNAFIS_ASSERT(temp[vmem::page_size() - 1] == add_offset(temp.ptr, temp.len - 1));
	SYNAFIS_ASSERT(temp.at(0) == temp[0]);
	SYNAFIS_ASSERT(temp.at(vmem::page_size() / 2) == temp[vmem::page_size() / 2]);
	SYNAFIS_ASSERT(temp.at(vmem::page_size() - 1) == temp[vmem::page_size() - 1]);
	bool threw{false};
	try {
		temp.at(vmem::page_size());
	} catch (std::out_of_range &) {
		threw = true;
	}
	if (!threw) { SYNAFIS_FAILURE("Out of bounds vmem::at should throw std::logic_error."); }
	threw = false;
	temp = nullptr;
	try {
		temp.at(0);
	} catch (std::logic_error &) {
		threw = true;
	}
	if (!threw) { SYNAFIS_FAILURE("vmem::at should throw std::logic_error when ptr == nullptr."); }
}

void t::protect(utc &)
{
	vmem temp{vmem::page_size() * 4, false};
	SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size()));
	SYNAFIS_ASSERT(no_access(temp, vmem::page_size(), vmem::page_size()));
	SYNAFIS_ASSERT(no_access(temp, vmem::page_size() * 2, vmem::page_size()));
	SYNAFIS_ASSERT(no_access(temp, vmem::page_size() * 3, vmem::page_size()));
	temp.readonly(vmem::page_size() + 1, vmem::page_size() * 2 - 2);
	SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size()));
	SYNAFIS_ASSERT(is_readonly(temp, vmem::page_size(), vmem::page_size()));
	SYNAFIS_ASSERT(is_readonly(temp, vmem::page_size() * 2, vmem::page_size()));
	SYNAFIS_ASSERT(no_access(temp, vmem::page_size() * 3, vmem::page_size()));
	temp.writable(vmem::page_size() * 2 - 1, 2);
	SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size()));
	SYNAFIS_ASSERT(is_writable(temp, vmem::page_size(), vmem::page_size()));
	SYNAFIS_ASSERT(is_writable(temp, vmem::page_size() * 2, vmem::page_size()));
	SYNAFIS_ASSERT(no_access(temp, vmem::page_size() * 3, vmem::page_size()));
	temp.forbid(vmem::page_size(), vmem::page_size());
	SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size()));
	SYNAFIS_ASSERT(no_access(temp, vmem::page_size(), vmem::page_size()));
	SYNAFIS_ASSERT(is_writable(temp, vmem::page_size() * 2, vmem::page_size()));
	SYNAFIS_ASSERT(no_access(temp, vmem::page_size() * 3, vmem::page_size()));
}

//!	\endcond

namespace {

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s() noexcept { return gc_test::vmem_suite; }

c protect{"protect", s(), pass, &t::protect};

c access{"access", s(), pass, &t::access};

c bounds{"bounds", s(), pass, &t::bounds};

c bool_convert{"bool_convert", s(), pass, &t::bool_convert};

c move_assign{"move assign", s(), pass, &t::move_assign};

c null_assign{"null assign", s(), pass, &t::null_assign};

c destruct{"destructor", s(), pass, &t::destruct};

c move_init{"move initialization", s(), pass, &t::move_init};

c reg_init{"regular initialization", s(), pass, &t::reg_init};

c def_init{"default initialization", s(), pass, &t::def_init};

c sane_page_size{"sane page size", s(), pass, &t::sane_page_size};

}
