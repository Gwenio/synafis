
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

/**	\file test/gc/vmem.cpp
 *	\brief Defines test cases for gc::vmem.
 */

#include "../../src/gc/vmem.hpp"
#include "../master.hpp"

#include <stdexcept>
#include <windows.h>

namespace unit_test {

/**	\class tester<gc::vmem>
 *	\brief Class with access to the interals of gc::vmem to make tests.
 */
template<>
class tester<gc::vmem> {
	/**	\fn tester()
	 *	\brief Deleted.
	 */
	tester() = delete;

	/**	\fn ~tester()
	 *	\brief Deleted.
	 */
	~tester() = delete;
public:
	/**	\typedef vmem
	 *	\brief Shorthand for the class being tested.
	 */
	typedef gc::vmem vmem;
private:
	/**	\fn invariants(vmem const& obj) noexcept
	 *	\brief Checks invariants for objects of the vmem class.
	 *	\param obj The object to check.
	 *	\returns Returns true if obj satisfy invariants.
	 */
	static bool invariants(vmem const& obj) noexcept {
		return !obj.ptr && (obj.len == 0) ||
			obj.ptr && (0 < obj.len) && is_allocated(obj);
	}

	/**	\fn no_access(vmem const& obj, std::size_t offset, std::size_t length) noexcept
	 *	\brief Checks that an area of memory in a vmem is unaccessible.
	 *	\param obj The object to check.
	 *	\param offset The starting point in the vmem.
	 *	\param length The size of the area to check.
	 *	\returns Returns true if the memory is unaccessible.
	 */
	static bool no_access(vmem const& obj, std::size_t offset, std::size_t length) noexcept {
		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(obj[offset], std::addressof(info), length);
		return info.State == MEM_COMMIT && info.Protect == PAGE_NOACCESS;
	}

	/**	\fn no_readonly(vmem const& obj, std::size_t offset, std::size_t length) noexcept
	 *	\brief Checks that an area of memory in a vmem is readonly.
	 *	\param obj The object to check.
	 *	\param offset The starting point in the vmem.
	 *	\param length The size of the area to check.
	 *	\returns Returns true if the memory is readonly.
	 */
	static bool is_readonly(vmem const& obj, std::size_t offset, std::size_t length) noexcept {
		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(obj[offset], std::addressof(info), length);
		return info.State == MEM_COMMIT && info.Protect == PAGE_READONLY;
	}

	/**	\fn no_writable(vmem const& obj, std::size_t offset, std::size_t length) noexcept
	 *	\brief Checks that an area of memory in a vmem is writable.
	 *	\param obj The object to check.
	 *	\param offset The starting point in the vmem.
	 *	\param length The size of the area to check.
	 *	\returns Returns true if the memory is writable.
	 */
	static bool is_writable(vmem const& obj, std::size_t offset, std::size_t length) noexcept {
		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(obj[offset], std::addressof(info), length);
		return info.State == MEM_COMMIT && info.Protect == PAGE_READWRITE;
	}

	/**	\fn is_allocated(vmem const& obj) noexcept
	 *	\brief Checks that an memory in a vmem is allocated.
	 *	\param obj The object to check.
	 *	\returns Returns true if the memory is allocated.
	 */
	static bool is_allocated(vmem const& obj) noexcept {
		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(obj.ptr, std::addressof(info), obj.len);
		return info.State == MEM_COMMIT;
	}

	/**	\fn is_free(void *addr, std::size_t length) noexcept
	 *	\brief Checks that an area of memory is free.
	 *	\param addr The start address.
	 *	\param length The size of the area.
	 *	\returns Returns true if the memory is free.
	 */
	static bool is_free(void *addr, std::size_t length) noexcept {
		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(addr, std::addressof(info), length);
		return info.State == MEM_FREE;
	}
public:
	/**	\fn sane_page_size(collector &)
	 *	\brief Test case checking the sanity of vmem::page_size.
	 */
	static void sane_page_size(collector &) {
		SYNAFIS_ASSERT(0 < vmem::page_size);
		SYNAFIS_ASSERT(alignof(std::max_align_t) < vmem::page_size);
		SYNAFIS_ASSERT(vmem::page_size % alignof(std::max_align_t) == 0);
	}

	/**	\fn def_init(collector &)
	 *	\brief Test case checking that the default contructor works.
	 */
	static void def_init(collector &) {
		vmem temp{};
		SYNAFIS_ASSERT(temp.ptr == nullptr);
		SYNAFIS_ASSERT(temp.len == 0);
	}

	/**	\fn reg_init(collector &)
	 *	\brief Test case checking normal constructor works.
	 */
	static void reg_init(collector &) {
		{
			vmem temp{vmem::page_size, true};
			SYNAFIS_ASSERT(temp.ptr != nullptr);
			SYNAFIS_ASSERT(temp.len == vmem::page_size);
			SYNAFIS_ASSERT(is_allocated(temp));
			SYNAFIS_ASSERT(is_writable(temp, 0, vmem::page_size));
		}
		{
			vmem temp{vmem::page_size, false};
			SYNAFIS_ASSERT(temp.ptr != nullptr);
			SYNAFIS_ASSERT(temp.len == vmem::page_size);
			SYNAFIS_ASSERT(is_allocated(temp));
			SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size));
		}
	}

	/**	\fn move_init(collector &)
	 *	\brief Test case checking them move constructor works.
	 */
	static void move_init(collector &) {
		vmem temp1{vmem::page_size, true};
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

	/**	\fn destruct(collector &)
	 *	\brief Test case checking the destructor frees memory.
	 */
	static void destruct(collector &) {
		void *addr{nullptr};
		{
			vmem temp{vmem::page_size, true};
			SYNAFIS_ASSERT(is_allocated(temp));
			addr = temp.ptr;
		}
		SYNAFIS_ASSERT(is_free(addr, vmem::page_size));
	}

	/**	\fn null_assign(collector &)
	 *	\brief Test case checking that assigning nullptr releases memory.
	 */
	static void null_assign(collector &) {
		vmem temp{vmem::page_size, true};
		SYNAFIS_ASSERT(temp.ptr != nullptr);
		SYNAFIS_ASSERT(invariants(temp));
		void *addr{temp.ptr};
		temp = nullptr;
		SYNAFIS_ASSERT(temp.ptr == nullptr);
		SYNAFIS_ASSERT(invariants(temp));
		SYNAFIS_ASSERT(is_free(addr, vmem::page_size));
	}

	/**	\fn move_assign(collector &)
	 *	\brief Test case checking that assigning nullptr releases memory.
	 */
	static void move_assign(collector &) {
		{
			vmem temp1{vmem::page_size, true};
			vmem temp2{};
			SYNAFIS_ASSERT(temp1.ptr != nullptr && invariants(temp1));
			SYNAFIS_ASSERT(temp2.ptr == nullptr && invariants(temp2));
			void *addr{temp1.ptr};
			temp1 = std::move(temp2);
			SYNAFIS_ASSERT(temp1.ptr == nullptr && invariants(temp1));
			SYNAFIS_ASSERT(temp2.ptr == nullptr && invariants(temp2));
			SYNAFIS_ASSERT(is_free(addr, vmem::page_size));
		}
		{
			vmem temp1{};
			vmem temp2{vmem::page_size, true};
			SYNAFIS_ASSERT(temp1.ptr == nullptr && invariants(temp1));
			SYNAFIS_ASSERT(temp2.ptr != nullptr && invariants(temp2));
			void *addr{temp2.ptr};
			temp1 = std::move(temp2);
			SYNAFIS_ASSERT(temp1.ptr == addr && invariants(temp1));
			SYNAFIS_ASSERT(temp2.ptr == nullptr && invariants(temp2));
		}
		{
			vmem temp1{vmem::page_size, true};
			vmem temp2{vmem::page_size, true};
			SYNAFIS_ASSERT(temp1.ptr != nullptr && invariants(temp1));
			SYNAFIS_ASSERT(temp2.ptr != nullptr && invariants(temp2));
			void *addr1{temp1.ptr};
			void *addr2{temp2.ptr};
			temp1 = std::move(temp2);
			SYNAFIS_ASSERT(temp1.ptr == addr2 && invariants(temp1));
			SYNAFIS_ASSERT(temp2.ptr == nullptr && invariants(temp2));
			SYNAFIS_ASSERT(is_free(addr1, vmem::page_size));
		}
		{
			vmem temp{vmem::page_size, true};
			SYNAFIS_ASSERT(temp.ptr != nullptr && invariants(temp));
			void *addr{temp.ptr};
			temp = std::move(temp);
			SYNAFIS_ASSERT(temp.ptr == addr && invariants(temp));
		}
	}

	/**	\fn bool_convert(collector &)
	 *	\brief Test case checking the boolean operations.
	 */
	static void bool_convert(collector &) {
		vmem temp1{};
		vmem temp2{vmem::page_size, true};
		SYNAFIS_ASSERT(!temp1);
		SYNAFIS_ASSERT(temp2);
	}

	/**	\fn bounds(collector &)
	 *	\brief Test case checking the boundary access members.
	 */
	static void bounds(collector &) {
		vmem temp1{};
		vmem temp2{vmem::page_size, true};
		SYNAFIS_ASSERT(temp1.begin() == nullptr);
		SYNAFIS_ASSERT(temp1.end() == nullptr);
		SYNAFIS_ASSERT(temp1.size() == 0);
		SYNAFIS_ASSERT(temp2.begin() == temp2.ptr);
		SYNAFIS_ASSERT(temp2.end() == gc::add_offset(temp2.ptr, temp2.len));
		SYNAFIS_ASSERT(temp2.size() == temp2.len);
	}

	/**	\fn access(collector &)
	 *	\brief Test case checking the offset access members.
	 */
	static void access(collector &) {
		vmem temp{vmem::page_size, true};
		SYNAFIS_ASSERT(temp[0] == temp.ptr);
		SYNAFIS_ASSERT(temp[vmem::page_size / 2] == gc::add_offset(temp.ptr, temp.len / 2));
		SYNAFIS_ASSERT(temp[vmem::page_size - 1] == gc::add_offset(temp.ptr, temp.len - 1));
		SYNAFIS_ASSERT(temp.at(0) == temp[0]);
		SYNAFIS_ASSERT(temp.at(vmem::page_size / 2) == temp[vmem::page_size / 2]);
		SYNAFIS_ASSERT(temp.at(vmem::page_size - 1) == temp[vmem::page_size - 1]);
		bool threw{false};
		try {
			temp.at(vmem::page_size);
		} catch(std::out_of_range &) {
			threw = true;
		}
		if (!threw) {
			fail_msg("Out of bounds vmem::at should throw std::logic_error.", __LINE__,
				__FILE__);
		}
		threw = false;
		temp = nullptr;
		try {
			temp.at(0);
		} catch(std::logic_error &) {
			threw = true;
		}
		if (!threw) {
			fail_msg("vmem::at should throw std::logic_error when ptr == nullptr.", __LINE__,
				__FILE__);
		}
	}

	/**	\fn protect(collector &)
	 *	\brief Test case checking the protections operations.
	 */
	static void protect(collector &) {
		vmem temp{vmem::page_size * 4, false};
		SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size));
		SYNAFIS_ASSERT(no_access(temp, vmem::page_size, vmem::page_size));
		SYNAFIS_ASSERT(no_access(temp, vmem::page_size * 2, vmem::page_size));
		SYNAFIS_ASSERT(no_access(temp, vmem::page_size * 3, vmem::page_size));
		temp.readonly(vmem::page_size + 1, vmem::page_size * 2 - 2);
		SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size) &&
			is_readonly(temp, vmem::page_size, vmem::page_size) &&
			is_readonly(temp, vmem::page_size * 2, vmem::page_size) &&
			no_access(temp, vmem::page_size * 3, vmem::page_size));
		temp.writable(vmem::page_size * 2 - 1, 2);
		SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size) &&
			is_writable(temp, vmem::page_size, vmem::page_size) &&
			is_writable(temp, vmem::page_size * 2, vmem::page_size) &&
			no_access(temp, vmem::page_size * 3, vmem::page_size));
		temp.forbid(vmem::page_size, vmem::page_size);
		SYNAFIS_ASSERT(no_access(temp, 0, vmem::page_size) &&
			no_access(temp, vmem::page_size, vmem::page_size) &&
			is_writable(temp, vmem::page_size * 2, vmem::page_size) &&
			no_access(temp, vmem::page_size * 3, vmem::page_size));
	}
};

}

namespace {

using t = unit_test::tester<gc::vmem>;
using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

static unit_test::suite s{"vmem", unit_test::gc_suite};

static c sane_page_size{"sane page size", s, pass, &t::sane_page_size};

static c def_init{"default initialization", s, pass, &t::def_init};

static c reg_init{"regular initialization", s, pass, &t::reg_init};

static c move_init{"move initialization", s, pass, &t::move_init};

static c destruct{"destructor", s, pass, &t::destruct};

static c null_assign{"null assign", s, pass, &t::null_assign};

static c move_assign{"move assign", s, pass, &t::move_assign};

static c bool_convert{"bool_convert", s, pass, &t::bool_convert};

static c bounds{"bounds", s, pass, &t::bounds};

static c access{"access", s, pass, &t::access};

static c protect{"protect", s, pass, &t::protect};

}
