
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

#ifndef SYNAFIS_GC_PTR_MATH_HPP
#define SYNAFIS_GC_PTR_MATH_HPP
#pragma once

/**	\file gc/ptr_math.hpp
 *	\brief Provides helpers for working with void pointers.
 */

#include <cstdint>

namespace gc {

/**	\fn add_offset(void *addr, std::uintptr_t offset) noexcept
 *	\brief Helper for pointer arithematic on void pointers.
 *	\param addr The address to add an offset to.
 *	\param offset The offset to add.
 *	\returns Returns the address + the offset.
 *	\details The type std::uintptr_t is required to be large enough to hold the
 *	\details address of any pointer type.
 *	\details Therefore casting pointers to and from that type is safe when
 *	\details using a standard compliant compiler and standard library combo.
 *	\warning Does not checked for overflow.
 */
inline constexpr void *add_offset(void *addr, std::uintptr_t offset) noexcept {
	return reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(addr) + offset);
}

/**	\fn sub_offset(void *addr, std::uintptr_t offset) noexcept
 *	\brief Helper for pointer arithematic on void pointers.
 *	\param addr The address to subtract an offset from.
 *	\param offset The offset to subtract.
 *	\returns Returns the address - the offset.
 *	\pre offset < reinterpret_cast<std::uintptr_t>(addr)
 *	\details The type std::uintptr_t is required to be large enough to hold the
 *	\details address of any pointer type.
 *	\details Therefore casting pointers to and from that type is safe when
 *	\details using a standard compliant compiler and standard library combo.
 *	\warning Does not checked for overflow.
 */
inline constexpr void *sub_offset(void *addr, std::uintptr_t offset) noexcept {
	return reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(addr) - offset);
}

/**	\fn sub_addr(void *addr, void *sub) noexcept
 *	\brief Helper for subtracting pointers.
 *	\param addr The address to subtract from.
 *	\param sub The address to subtract.
 *	\returns Returns addr - sub.
 *	\details The type ptrdiff_t is meant for holding the difference between pointers.
 *	\warning Does not checked for overflow.
 */
inline constexpr std::ptrdiff_t sub_addr(void *addr, void *sub) noexcept {
	if (sub <= addr) {
		return reinterpret_cast<std::uintptr_t>(addr) - reinterpret_cast<std::uintptr_t>(sub);
	} else {
		std::ptrdiff_t temp{reinterpret_cast<std::uintptr_t>(sub) - reinterpret_cast<std::uintptr_t>(addr)};
		return -temp;
	}
}

}

#endif
