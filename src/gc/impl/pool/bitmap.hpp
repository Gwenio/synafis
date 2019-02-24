
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

/**	\file src/gc/impl/pool/bitmap.hpp
 *	\brief Defines the type tracking the state of slots with a bit flag.
 *	\ingroup gc_pool
 */

#include "../../../unit_test.hpp"

#ifndef SYNAFIS_GC_POOL_BITMAP_HPP
#define SYNAFIS_GC_POOL_BITMAP_HPP
#pragma once

#include <bitset>
#include <tuple>

namespace gc {

class pool;

/**	\class bitmap
 *	\brief Type for tracking slot state with bit flags.
 *	\ingroup gc_pool
 */
class bitmap
{
	//!	\cond friends
	friend unit_test::tester<bitmap>;
	friend unit_test::tester<pool>;
	//!	\endcond

public:
	/**	\typedef group
	 *	\brief Type for an element in bitmap state tracking arrays.
	 */
	using group = typename std::bitset<(sizeof(std::uintptr_t) * 8)>;

private:
	/**	\var data
	 *	\brief Points to the beginning of the bitmap.
	 */
	group *data;

	/**	\fn locate(std::size_t offset) const noexcept
	 *	\brief Locates the group and bit for an offset.
	 *	\param offset The offset to get the group and bit for.
	 *	\returns Returns the group and bit for offset.
	 */
	std::tuple<group &, std::size_t> locate(std::size_t offset) const noexcept
	{
		std::size_t const s{bits()};
		return std::tuple<group &, std::size_t>{data[offset / s], offset % s};
	}

public:
	/**	\fn bitmap() noexcept
	 *	\brief Default constructor.
	 */
	constexpr bitmap() noexcept : data(nullptr) {}

	/**	\fn bitmap(bitmap const &other) noexcept
	 *	\brief Copy constructor.
	 *	\param other The bitmap to copy.
	 */
	bitmap(bitmap const &other) noexcept : data(other.data) {}

	/**	\fn bitmap(group *ptr) noexcept
	 *	\brief Constructs a bitmap with data = ptr.
	 *	\param ptr The address for data.
	 */
	bitmap(group *ptr) noexcept : data(ptr) {}

	/**	\fn ~bitmap() noexcept
	 *	\brief Default destructor.
	 */
	~bitmap() noexcept = default;

	/**	\fn operator=(bitmap const &other) noexcept
	 *	\brief Deleted.
	 *	\returns *this
	 */
	bitmap &operator=(bitmap const &other) noexcept
	{
		data = other.data;
		return *this;
	}

	/**	\fn begin() noexcept
	 *	\brief Gets an iterator to the first group of bits.
	 *	\returns An iterator to the first group of bits.
	 */
	group *begin() noexcept { return data; }

	/**	\fn cbegin() noexcept
	 *	\brief Gets an iterator to the first group of bits.
	 *	\returns An iterator to the first group of bits.
	 */
	group const *cbegin() const noexcept { return data; }

	/**	\fn test(std::size_t offset) const noexcept
	 *	\brief Checks the bit at offset.
	 *	\param offset The bit to check.
	 *	\returns The value of the bit.
	 */
	bool test(std::size_t offset) const noexcept
	{
		auto const x = locate(offset);
		return std::get<0>(x).test(std::get<1>(x));
	}

	/**	\fn set(std::size_t offset) noexcept
	 *	\brief Sets the bit at offset to true.
	 *	\param offset The bit to set.
	 */
	void set(std::size_t offset) noexcept
	{
		auto const x = locate(offset);
		std::get<0>(x).set(std::get<1>(x));
	}

	/**	\fn reset(std::size_t offset) noexcept
	 *	\brief Sets the bit at offset to false.
	 *	\param offset The bit to reset.
	 */
	void reset(std::size_t offset) noexcept
	{
		auto const x = locate(offset);
		std::get<0>(x).reset(std::get<1>(x));
	}

	/**	\fn placement(std::size_t offset) noexcept
	 *	\brief Calculates where to place the beginning of a bitmap.
	 *	\param offset The offset from a page boundary to place the bitmap after.
	 *	\returns Returns the offset to place the bitmap at relative to the same point offset
	 *	\returns was based on.
	 *	\post The returned value will be >= offset.
	 */
	static constexpr std::size_t placement(std::size_t offset) noexcept
	{
		std::size_t const a{alignof(group)};
		return ((offset % a == 0) ? offset : ((offset / a) + 1) * a);
	}

	/**	\fn bits() noexcept
	 *	\brief Gets the number of bits in a group.
	 *	\returns The number of bits in a group.
	 */
	static constexpr std::size_t bits() noexcept { return group{}.size(); }

	/**	\fn length(std::size_t capacity) noexcept
	 *	\brief Gets the number of groups in a bitmap for capacity slots.
	 *	\param capacity The number of bits in the bitmap.
	 *	\returns Returns the size of a bitmap with capacity bits in terms of groups.
	 */
	static constexpr std::size_t length(std::size_t capacity) noexcept
	{
		std::size_t const size{bits()};
		return (capacity / size) + (capacity % size == 0 ? 0 : 1);
	}

	/**	\fn footprint(std::size_t capacity) noexcept
	 *	\brief Gets the size of the memory occupied by the bitmap in bytes.
	 *	\param capacity The number of bits in the bitmap.
	 *	\returns Returns the size of a bitmap in bytes.
	 */
	static constexpr std::size_t footprint(std::size_t capacity) noexcept
	{
		return length(capacity) * sizeof(group);
	}
};

}

#endif
