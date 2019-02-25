
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

/**	\file test/gc/vmem/winapi.cpp
 *	\brief Defines helpers for testing gc::vmem specific to Windows.
 */

#include "../vmem.hpp"
#include "../../../src/gc/impl/vmem.hpp"

#include <windows.h>

using gc::vmem;

//!	\cond impl_details

using t = typename unit_test::tester<vmem>;
using utc = typename unit_test::collector;

bool t::no_access(vmem const &obj, std::size_t offset, std::size_t length) noexcept
{
	MEMORY_BASIC_INFORMATION info;
	memset(std::addressof(info), 0, sizeof(info));
	VirtualQuery(obj[offset], std::addressof(info), sizeof(info));
	return (info.Protect & PAGE_NOACCESS) == PAGE_NOACCESS;
}

bool t::is_readonly(vmem const &obj, std::size_t offset, std::size_t length) noexcept
{
	MEMORY_BASIC_INFORMATION info;
	memset(std::addressof(info), 0, sizeof(info));
	VirtualQuery(obj[offset], std::addressof(info), sizeof(info));
	return (info.Protect & PAGE_READONLY) == PAGE_READONLY;
}

bool t::is_writable(vmem const &obj, std::size_t offset, std::size_t length) noexcept
{
	MEMORY_BASIC_INFORMATION info;
	memset(std::addressof(info), 0, sizeof(info));
	VirtualQuery(obj[offset], std::addressof(info), sizeof(info));
	return (info.Protect & PAGE_READWRITE) == PAGE_READWRITE;
}

bool t::is_allocated(vmem const &obj) noexcept
{
	MEMORY_BASIC_INFORMATION info;
	memset(std::addressof(info), 0, sizeof(info));
	VirtualQuery(obj.ptr, std::addressof(info), sizeof(info));
	return (info.State & MEM_COMMIT) == MEM_COMMIT;
}

bool t::is_free(void *addr, std::size_t length) noexcept
{
	MEMORY_BASIC_INFORMATION info;
	memset(std::addressof(info), 0, sizeof(info));
	VirtualQuery(addr, std::addressof(info), sizeof(info));
	return (info.State & MEM_FREE) == MEM_FREE;
}

//!	\endcond
