
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

#ifndef SYNAFIS_GC_POOL_HPP
#define SYNAFIS_GC_POOL_HPP
#pragma once

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

#ifndef SYNAFIS_GC_VMEM_HPP
#include "vmem.hpp"
#endif

/**	\file gc/pool.hpp
 *	\brief Defines the type for managing a memory pool.
 */

namespace gc {

/**	\class pool
 *	\brief Type to manage a pool of fixed size memory slots.
 */
class pool {
	friend unit_test::tester<pool>;

	/**	\fn pool()
	 *	\brief Deleted.
	 */
	pool() = delete;

	/**	\fn pool(pool const&)
	 *	\brief Deleted.
	 */
	pool(pool const&) = delete;

	/**	\fn pool(pool &&)
	 *	\brief Deleted.
	 */
	pool(pool &&) = delete;

	/**	\fn operator=(pool const&)
	 *	\brief Deleted.
	 */
	pool &operator=(pool const&) = delete;

	/**	\fn operator=(pool &&)
	 *	\brief Deleted.
	 */
	pool &operator=(pool &&) = delete;
public:
	/**	\typedef bit_group
	 *	\brief Type for an element in bitmap state tracking arrays.
	 */
	typedef std::bitset<(sizeof(std::uintptr_t) * 8)> bit_group;
private:
	/**	\class node
	 *	\brief Used to form a stack of free locations.
	 *	\details A node is located at the start of the corresponding free locations;
	 *	\details thus, the location can be retrieve casting a pointer to the node to
	 *	\details a void pointer.
	 *	\details
	 *	\details This method of tracking free objects makes all allocations and
	 *	\details deallocations take O(1) time.
	 *	\details This does not mean it is the most effiecint; rather, it means
	 *	\details times are reliable.
	 */
	struct node {
	public:
		/**	\var next
		 *	\brief The next free location.
		 */
		node *next;
	};

	/**	\var region
	 *	\brief The memory region of the pool.
	 *	\note The intent is for the pool to be allocated in the block of region.
	 */
	vmem region;

	/**	\var unit
	 *	\brief The allocation unit size.
	 *	\note At least the sizeof the type being allocated and a multiple of its alignof.
	 */
	std::uintptr_t const unit;

	/**	\var type
	 *	\brief The identity of the type being allocated.
	 *	\todo Maybe this should just be the finalizer of the type.
	 */
	identity const& type;

	/**	\var free
	 *	\brief The next free object to allocate. Acts as a stack.
	 *	\note If free == nullptr, there is no free memory to allocate.
	 */
	node *free;

	/**	\var space
	 *	\brief The number of free slots.
	 *	\invariant If free == nullptr then space == 0.
	 */
	std::size_t space;

	/**	\var bitmap
	 *	\brief Tracks allocated slots. Is a pointer to an array.
	 *	\details While nodes track free objects for allocation, the bitmap is
	 *	\details needed to know if the finalizer should be called on a slot
	 *	\details that is not marked. Traversing the free list takes longer
	 *	\details than checking a know bit flag in terms of big O notation.
	 *	\details
	 *	\details The free stack is better than the bit map for finding an
	 *	\details unknown free slot to allocate.
	 *	\todo The bitmap is not needed for types without a finalizer.
	 */
	bit_group *bitmap;

	/**	\var colors
	 *	\brief Tracks reachable objects. Is a pointer to an array.
	 *	\note Slots with their matching bit set are called 'black' while
	 *	\note the others are 'white'.
	 */
	bit_group *colors;

	/**	\var slots
	 *	\brief The start of the address range to allocate objects from.
	 */
	void *const slots;

	/**	\var end
	 *	\brief The end of the address range to allocate objects from.
	 *	\invariant Equal to slots + (unit * <number of slots in the pool>).
	 */
	void *const end;

	/**	\fn deallocate(void *ptr) noexcept
	 *	\brief Preforms bookkeeping related to freeing a slot.
	 *	\param ptr The object to deallocate.
	 *	\pre 'ptr' must equal slots + a multiple of unit.
	 *	\pre slots <= ptr < end
	 *	\details Calls the finalizer on the object a ptr if there is one in type.
	 *	\details
	 *	\details The address becomes the top of the free stack, storing the old top
	 *	\details as the next node.
	 *	\details
	 *	\details Increments space.
	 *	\details
	 *	\details Does not update the bitmap, that piece of bookkeeping is done by
	 *	\details the caller. This is so sweep can update bitmap in batches.
	 */
	void deallocate(void *ptr) noexcept;

	/**	\fn pool(vmem && mem, identity const& id, std::size_t cap, std::size_t u, void *start) noexcept
	 *	\brief Called by create to initialize a new pool.
	 *	\param mem The virtual memory the pool is allocated on is to be owned by the pool.
	 *	\param id The type of object placed in the pool.
	 *	\param cap The maximum capacity of the pool.
	 *	\param u The unit size of slots in the pool.
	 *	\param start The starting address for the memory to allocate objects from.
	 *	\details Sets the bits in bitmap and colors to false.
	 *	\details Places all slots in the free stack.
	 */
	pool(vmem && mem, identity const& id, std::size_t cap, std::size_t u, void *start) noexcept;
public:
	/**	\fn ~pool() noexcept
	 *	\brief Finalizes all objects and frees region.
	 *	\note Pools are to have their destructor called directly.
	 */
	~pool() noexcept;

	/**	\fn create(identity const& id, std::size_t unit) noexcept
	 *	\brief Creates a new pool.
	 *	\param id The type the pool allocates memory for.
	 *	\param unit The size of the unit of allocation.
	 *	\returns Returns a new pool or nullptr if virtual memory could not be allocated.
	 *	\note To deallocate the pool, directly call its destructor.
	 */
	static pool *create(identity const& id, std::size_t unit) noexcept;

	/**	\fn allocate() noexcept
	 *	\brief Allocates a free slot.
	 *	\returns Returns a previously free slot or nullptr is there was not one.
	 *	\details Reduces space by one.
	 */
	void *allocate() noexcept;

	/**	\fn mark(void *ptr) noexcept
	 *	\brief Marks an object as reachable so it will not be deallocate be sweep.
	 *	\param ptr The object to mark as reachable.
	 *	\pre slots <= ptr < end
	 *	\details Sets the appropriate bit in the color array.
	 */
	void mark(void *ptr) noexcept;

	/**	\fn from(void *ptr) const noexcept
	 *	\brief Checks if a pointer is from the pool.
	 *	\param ptr The object to check.
	 *	\returns slots <= ptr < end
	 */
	bool from(void *ptr) const noexcept {
		return slots <= ptr && ptr < end;
	}

	/**	\fn sweep() noexcept
	 *	\brief Deallocates all unmarked objects.
	 *	\param ptr The object to mark as reachable.
	 *	\details Sets the appropriate bit in the color array.
	 */
	void sweep() noexcept;

	/**	\fn available() const noexcept
	 *	\brief Gets the number of free slots.
	 *	\returns space
	 */
	std::size_t available() const noexcept {
		return space;
	}
};

}

#endif
