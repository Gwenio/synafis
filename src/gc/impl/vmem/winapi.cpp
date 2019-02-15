
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

/**	\file src/gc/impl/vmem/winapi.cpp
 *	\note "WIN32_LEAN_AND_MEAN" might need to be defined if there are errors
 *	\note in the Windows system headers.
 *	\note
 *	\note Or for errors in an editor rather than a compiler, the architecture
 *	\note preprocessor definitions the compiler is expected to provide might
 *	\note not be present.
 */

#include "../vmem.hpp"

#include <stdexcept>
#include <windows.h>

namespace {

/**	\fn get_page_size() noexcept
 *	\brief Gets the value for vmem::page_size.
 *	\returns Returns the system page size.
 */
inline std::size_t get_page_size() noexcept
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwPageSize;
}

}

namespace gc {

std::size_t const vmem::page_size{get_page_size()};

void *vmem::allocate(std::size_t size, bool access) noexcept
{
	SYNAFIS_ASSERT(size > 0);
	return VirtualAlloc(
		nullptr, size, MEM_COMMIT | MEM_RESERVE, access ? PAGE_READWRITE : PAGE_NOACCESS);
}

void vmem::deallocate(void *ptr) noexcept
{
	SYNAFIS_ASSERT(ptr != nullptr);
	VirtualFree(ptr, 0, MEM_RELEASE);
}

void *vmem::operator[](std::size_t offset) const noexcept
{
	SYNAFIS_ASSERT(ptr != nullptr);
	SYNAFIS_ASSERT(offset < len);
	return add_offset(ptr, offset);
}

void *vmem::at(std::size_t offset) const
{
	if (ptr) {
		if (offset < len) {
			return add_offset(ptr, offset);
		} else {
			throw std::out_of_range{"Precondition (offset < len) violated."};
		}
	} else {
		throw std::logic_error{"Precondition (ptr != nullptr) violated."};
	}
}

bool vmem::forbid(std::size_t offset, std::size_t length)
{
	void *temp{(*this)[offset]};
	if ((offset + length) > len) {
		throw std::out_of_range{"Precondition (offset + length > len) violated."};
	}
	DWORD old{0};
	return VirtualProtect(temp, length, PAGE_NOACCESS, &old);
}

bool vmem::readonly(std::size_t offset, std::size_t length)
{
	void *temp{(*this)[offset]};
	if ((offset + length) > len) {
		throw std::out_of_range{"Precondition (offset + length > len) violated."};
	}
	DWORD old{0};
	return VirtualProtect(temp, length, PAGE_READONLY, &old);
}

bool vmem::writable(std::size_t offset, std::size_t length)
{
	void *temp{(*this)[offset]};
	if ((offset + length) > len) {
		throw std::out_of_range{"Precondition (offset + length > len) violated."};
	}
	DWORD old{0};
	return VirtualProtect(temp, length, PAGE_READWRITE, &old);
}

}
