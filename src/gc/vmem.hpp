
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

#ifndef SYNAFIS_GC_VMEM_HPP
#define SYNAFIS_GC_VMEM_HPP
#pragma once

#include <utility>

#ifndef SYNAFIS_UNIT_TEST_HPP
#include "../unit_test.hpp"
#endif

#ifndef SYNAFIS_GC_PTR_MATH_HPP
#include "ptr_math.hpp"
#endif

/**	\file gc/vmem.hpp
 *	\brief Defines an abstraction layer for the host's virtual memory system.
 */

namespace gc {

/**	\class vmem
 *	\brief Abstracts the host system's virtual memory.
 *	\todo Investigate supporting huge pages to reduce page map size in the kernel.
 *	\note Execute privalage is considered unneeded at this time.
 */
class vmem {
	friend unit_test::tester<vmem>;
private:
	/**	\var ptr
	 *	\brief The address of the virtual memory.
	 */
	void *ptr;

	/**	\var len
	 *	\brief The size of the virtual memory.
	 */
	std::size_t len;

	/**	\fn allocate(std::size_t size, bool access) noexcept
	 *	\brief Allocates virtual memory.
	 *	\param size The size of the block of memory to allocate.
	 *	\param access If true start with read and write access; otherwise, start with no access.
	 *	\returns Returns the allocate memory or nullptr on failure.
	 *	\pre 'size' must be greater than zero.
	 *	\note If size is not a multiple of page_size, it will be rounded up to the nearest multiple.
	 */
	static void *allocate(std::size_t size, bool access) noexcept;

	/**	\fn deallocate(void *ptr) noexcept
	 *	\brief Dellocates virtual memory.
	 *	\param ptr The virtual memory to deallocate.
	 *	\pre 'ptr' must be a block of virtual memory returned by allocate.
	 */
	static void deallocate(void *ptr) noexcept;
public:
	/**	\var page_size
	 *	\brief The basic unit size of virtual memory blocks.
	 */
	static std::size_t const page_size;

	/**	\fn vmem() noexcept
	 *	\brief Initializes with no virtual memory.
	 */
	constexpr vmem() noexcept : ptr(nullptr), len(0) {}

	/**	\fn (std::size_t const s, bool const access) noexcept
	 *	\brief Initializes with a new virtual memory block.
	 *	\param s The size of the block.
	 *	\param access If true start with read and write access; otherwise, start with no access.
	 */
	vmem(std::size_t const s, bool const access) noexcept : vmem() {
		ptr = allocate(s, access);
		if (ptr) {
			len = s;
		}
	}

	/**	\fn vmem(vmem const&)
	 *	\brief Deleted.
	 */
	vmem(vmem const&) = delete;

	/**	\fn vmem(vmem && other) noexcept
	 *	\brief Moves the virtual memory from other to this.
	 */
	constexpr vmem(vmem && other) noexcept : vmem() {
		if (other.ptr) {
			ptr = std::exchange(other.ptr, nullptr);
			len = std::exchange(other.len, 0);
		}
	}

	/**	\fn ~vmem() noexcept
	 *	\brief Deallocates the virtual memory if ptr is not nullptr.
	 */
	~vmem() noexcept {
		if (ptr) {
			deallocate(ptr);
		}
	}

	/**	\fn operator=(nullptr_t const&) noexcept
	 *	\brief Removes the owned virtual memory if this has any.
	 */
	constexpr vmem &operator=(nullptr_t const&) noexcept {
		if (ptr) {
			deallocate(ptr);
			ptr = nullptr;
			len = 0;
		}
		return *this;
	}

	/**	\fn operator=(vmem const&)
	 *	\brief Deleted.
	 */
	vmem &operator=(vmem const&) = delete;

	/**	\fn vmem(vmem && other) noexcept
	 *	\brief Moves the virtual memory from other to this.
	 */
	constexpr vmem &operator=(vmem && other) noexcept {
		if (other.ptr) {
			if (ptr) {
				deallocate(ptr);
			}
			ptr = std::exchange(other.ptr, nullptr);
			len = std::exchange(other.len, 0);
			return *this;
		} else {
			return *this = nullptr;
		}
	}

	/**	\fn operator bool() const noexcept
	 *	\brief Converts to bool.
	 *	\returns Returns true if ptr != nullptr.
	 */
	constexpr operator bool() const noexcept {
		return ptr != nullptr;
	}

	/**	\fn operator!() const noexcept
	 *	\brief Converts to bool.
	 *	\returns Returns true if ptr == nullptr.
	 */
	constexpr bool operator!() const noexcept {
		return ptr == nullptr;
	}

	/**	\fn size() const noexcept
	 *	\brief Gets the size of the owned virtual memory.
	 *	\returns The size of the virtual memory.
	 */
	constexpr std::size_t size() const noexcept {
		return len;
	}

	/**	\fn begin() const noexcept
	 *	\brief Gets the starting address of the block.
	 *	\returns ptr
	 */
	constexpr void *begin() const noexcept {
		return ptr;
	}

	/**	\fn end() const noexcept
	 *	\brief Gets the end address of the block.
	 *	\returns The address just past the end.
	 *	\returns Returns nullptr if ptr is nullptr.
	 */
	constexpr void *end() const noexcept {
		return ptr ? add_offset(ptr, len) : nullptr;
	}

	/**	\fn operator[](std::size_t offset) const noexcept
	 *	\brief Gets the pointer for an offset into the virtual memory.
	 *	\param offset The offset into the block to get a pointer for.
	 *	\returns The pointer to the offset from the beginning of the block.
	 *	\returns Returns nullptr if ptr is nullptr.
	 *	\pre offset < len
	 *	\warning The precondition is only checked with SYNAFIS_ASSERT,
	 *	\warning which does nothing when not debugging or testing.
	 */
	void *operator[](std::size_t offset) const noexcept;

	/**	\fn at(std::size_t offset) const
	 *	\brief Gets the pointer for an offset into the virtual memory.
	 *	\param offset The offset into the block to get a pointer for.
	 *	\returns The pointer to the offset from the beginning of the block.
	 *	\throws Throws std::logic_error if any of the preconditions are violated.
	 *	\pre offset < len
	 *	\pre ptr != nullptr
	 */
	void *at(std::size_t offset) const;

	/**	\fn forbid(std::size_t offset, std::size_t length) noexcept
	 *	\brief Makes part of a previously allocated block unaccessible.
	 *	\param offset The starting point in the virtual memory block.
	 *	\param length The size of area to forbid.
	 *	\returns Returns a boolean value indicated whether the change succeeded.
	 *	\pre The whole indicated area must reside in the owned virtual memory.
	 *	\pre ptr != nullptr
	 *	\warning If any portion of a page falls in the area, the whole page
	 *	\warning will have its protection settings changed.
	 */
	bool forbid(std::size_t offset, std::size_t length) noexcept;

	/**	\fn readonly(std::size_t offset, std::size_t length) noexcept
	 *	\brief Makes part of a previously allocated block have read access only.
	 *	\param offset The starting point in the virtual memory block.
	 *	\param length The size of area to have read access only.
	 *	\returns Returns a boolean value indicated whether the change succeeded.
	 *	\pre The whole indicated area must reside in the owned virtual memory.
	 *	\pre ptr != nullptr
	 *	\warning If any portion of a page falls in the area, the whole page
	 *	\warning will have its protection settings changed.
	 */
	bool readonly(std::size_t offset, std::size_t length) noexcept;

	/**	\fn writable(std::size_t offset, std::size_t length) noexcept
	 *	\brief Makes part of a previously allocated block have read and write access.
	 *	\param offset The starting point in the virtual memory block.
	 *	\param length The size of area to make writable.
	 *	\returns Returns a boolean value indicated whether the change succeeded.
	 *	\pre The whole indicated area must reside in the owned virtual memory.
	 *	\pre ptr != nullptr
	 *	\warning If any portion of a page falls in the area, the whole page
	 *	\warning will have its protection settings changed.
	 */
	bool writable(std::size_t offset, std::size_t length) noexcept;
};

}

#endif
