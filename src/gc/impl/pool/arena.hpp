
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

/**	\file src/gc/impl/pool/arena.hpp
 *	\brief Defines the type for managing the area of memory containing a pool's slots.
 *	\ingroup gc_pool
 */

#include "../../../unit_test.hpp"

#ifndef SYNAFIS_GC_POOL_ARENA_HPP
#define SYNAFIS_GC_POOL_ARENA_HPP
#pragma once

namespace gc {

class pool;

/**	\class arena
 *	\brief Manages the area of memory containing a pool's slots.
 *	\ingroup gc_pool
 */
class arena
{
	//!	\cond friends
	friend unit_test::tester<arena>;
	friend unit_test::tester<pool>;
	//!	\endcond

	/**	\fn arena()
	 *	\brief Deleted.
	 */
	arena() = delete;

	/**	\fn operator=(arena const &)
	 *	\brief Deleted.
	 */
	arena &operator=(arena const &) = delete;

	/**	\fn operator=(arena &&)
	 *	\brief Deleted.
	 */
	arena &operator=(arena &&) = delete;

private:
	/**	\var capacity
	 *	\brief The number of slots in the pool.
	 */
	std::size_t const capacity;

	/**	\var unit
	 *	\brief The allocation unit max.
	 *	\note At least the sizeof the type being allocated and a multiple of its alignof.
	 */
	std::uintptr_t const unit;

	/**	\var front
	 *	\brief The start of the address range to allocate objects from.
	 */
	void *const front;

	/**	\var back
	 *	\brief The end of the address range to allocate objects from.
	 *	\invariant Equal to slots + (unit * \<number of slots in the pool\>).
	 */
	void *const back;

public:
	/**	\fn arena(arena const &other) noexcept
	 *	\brief Copy constructor.
	 *	\param other The other arena to copy.
	 */
	constexpr arena(arena const &other) noexcept :
		capacity(other.capacity), unit(other.unit), front(other.front), back(other.back)
	{}

	/**	\fn arena(void *begin, std::size_t cap, std::uintptr_t u) noexcept
	 *	\brief Constructor.
	 *	\param begin The beginning of the arena.
	 *	\param cap The number of slots in the arena.
	 *	\param u The size of a slot for the arena.
	 */
	arena(void *begin, std::size_t cap, std::uintptr_t u) noexcept :
		front(begin), back(static_cast<std::byte *>(begin) + (cap * u)), capacity(cap), unit(u)
	{}

	/**	\fn ~arena() noexcept
	 *	\brief Default destructor.
	 */
	~arena() noexcept = default;

	/**	\fn operator[](std::size_t offset) noexcept
	 *	\brief Gets a pointer to the slot at offset.
	 *	\param offset The offset of the slot to get a pointer to.
	 *	\returns Returns a pointer to the slot at offset.
	 *	\pre offset < max()
	 */
	void *operator[](std::size_t offset) noexcept;

	/**	\fn begin() const noexcept
	 *	\brief Gets the start of the arena.
	 *	\returns front
	 */
	void *begin() noexcept { return front; }

	/**	\fn end() const noexcept
	 *	\brief Gets the address passed the end of the arena.
	 *	\returns back
	 */
	void *end() noexcept { return back; }

	/**	\fn cbegin() const noexcept
	 *	\brief Gets the start of the arena.
	 *	\returns front
	 */
	void *cbegin() const noexcept { return front; }

	/**	\fn cend() const noexcept
	 *	\brief Gets the address passed the end of the arena.
	 *	\returns back
	 */
	void *cend() const noexcept { return back; }

	/**	\fn get_slot(void *slot) const noexcept
	 *	\brief Gets the offset of a slot.
	 *	\param slot A pointer to an address within a slot.
	 *	\returns Returns the offset of the slot.
	 *	\pre from(slot)
	 */
	std::size_t get_slot(void *slot) const noexcept;

	/**	\fn location() const noexcept
	 *	\brief Gets an address to compare sources for sorting.
	 *	\returns An address used to compare sources for sorting.
	 */
	void *location() const noexcept { return front; }

	/**	\fn max() const noexcept
	 *	\brief Gets the number of slots in the arena.
	 *	\returns capacity
	 */
	std::size_t max() const noexcept { return capacity; }

	/**	\fn size() const noexcept
	 *	\brief Gets the size of a slot for the arena.
	 *	\returns unit
	 */
	std::uintptr_t size() const noexcept { return unit; }

	/**	\fn from(void *slot) const noexcept
	 *	\brief Checks if a pointer is from the arena.
	 *	\param slot The address to check.
	 *	\returns front <= slot < back
	 */
	bool from(void *slot) const noexcept { return front <= slot && slot < back; }
};

}

#endif
