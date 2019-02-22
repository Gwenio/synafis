
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

/**	\defgroup gc_pool Pool
 *	\brief Group for gc::pool and its sub-components.
 *	\ingroup gc_impl
 */

/**	\dir src/gc/impl/pool
 *	\brief Contains sub-components of gc::pool.
 *	\ingroup gc_pool
 */

/**	\file src/gc/impl/pool.hpp
 *	\brief Defines the type for managing a memory pool.
 *	\ingroup gc_pool
 */

#include "../../unit_test.hpp"
#include "vmem.hpp"
#include "source.hpp"

#ifndef SYNAFIS_GC_POOL_HPP
#define SYNAFIS_GC_POOL_HPP
#pragma once

#include "pool/free_list.hpp"
#include "pool/gray_list.hpp"
#include "pool/bitmap.hpp"
#include "pool/arena.hpp"
#include "pool/blueprint.hpp"

#include <list>

namespace gc {

/**	\class pool
 *	\brief Type to manage a pool of fixed size memory slots.
 *	\ingroup gc_pool
 */
class pool : public isource
{
	//!	\cond friends
	friend unit_test::tester<pool>;
	//!	\endcond

	/**	\fn pool()
	 *	\brief Deleted.
	 */
	pool() = delete;

	/**	\fn pool(pool const &)
	 *	\brief Deleted.
	 */
	pool(pool const &) = delete;

	/**	\fn pool(pool &&)
	 *	\brief Deleted.
	 */
	pool(pool &&) = delete;

	/**	\fn operator=(pool const &)
	 *	\brief Deleted.
	 */
	pool &operator=(pool const &) = delete;

	/**	\fn operator=(pool &&)
	 *	\brief Deleted.
	 */
	pool &operator=(pool &&) = delete;

public:
	/**	\typedef bit_group
	 *	\brief Type for an element in bitmap state tracking arrays.
	 */
	using bit_group = typename bitmap::group;

private:
	/**	\var region
	 *	\brief The memory region of the pool.
	 *	\note The intent is for the pool to be allocated in the block of region.
	 */
	vmem region;

	/**	\var type
	 *	\brief The identity of the type being allocated.
	 *	\todo Maybe this should just be the finalizer of the type.
	 */
	identity const &type;

	/**	\var tracking
	 *	\brief A list of soft_ptr::data objects being tracked.
	 */
	std::list<soft_ptr::data *> tracking;

	/**	\var free
	 *	\brief The list of free slots.
	 *	\invariant free.space <= capacity
	 */
	free_list free;

	/**	\var initialized
	 *	\brief Tracks allocated slots. Is a pointer to an array.
	 *	\details While nodes track free objects for allocation, the bitmap is
	 *	\details needed to know if the finalizer should be called on a slot
	 *	\details that is not marked.
	 *	\todo The bitmap is not needed for types without a finalizer.
	 *	\note Could be done with a node list similar to free objects;
	 *	\note however, if the pointer cannot be squeezed in after the
	 *	\note object then it would take more memory than a bitmap.
	 */
	bitmap initialized;

	/**	\var reachable
	 *	\brief Tracks reachable objects. Is a pointer to an array.
	 *	\note Slots with their matching bit set are called 'black' while
	 *	\note the others are 'white'.
	 */
	bitmap reachable;

	/**	\var gray
	 *	\brief The list of gray slots pending traversal.
	 */
	gray_list gray;

	/**	\var store
	 *	\brief The arena for the pool's slots.
	 */
	arena store;

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

	/**	\fn pool(identity const &id, blueprint const &cfg, vmem &&mem) noexcept
	 *	\brief Interprets the information in cfg.
	 *	\param id The type the pool allocates memory for.
	 *	\param cfg The blueprint for the pool.
	 *	\param mem The virtual memory used for the pool.
	 */
	pool(identity const &id, blueprint const &cfg, vmem &&mem) noexcept;

	/**	\fn pool(identity const &id, vmem &&m, arena const &s, void **g, void *b) noexcept
	 *	\brief The final step in creating a pool.
	 *	\param id The type the pool allocates memory for.
	 *	\param m The virtual memory used for the pool.
	 *	\param s The arena for store to copy.
	 *	\param g The value to initialize 'gray' with.
	 *	\param b The address for the start of the bitmaps.
	 *	\details Sets the bits in bitmap and colors to false.
	 *	\details Places all slots in the free stack.
	 *	\pre The value of 'g' must be nullptr if the type is not traversable.
	 *	\pre The value of 'g' must be a valid pointer if the type is traversable.
	 */
	pool(identity const &id, vmem &&m, arena const &s, void **g, void *b) noexcept;

public:
	/**	\fn pool(identity const &id, blueprint const &cfg)
	 *	\brief Creates a new pool.
	 *	\param id The type the pool allocates memory for.
	 *	\param cfg The blueprint for the pool.
	 *	\throws std::bad_alloc if virtual memory could not be allocated.
	 */
	pool(identity const &id, blueprint const &cfg);

	/**	\fn ~pool() noexcept final
	 *	\brief Finalizes all objects and frees region.
	 *	\note Pools are to have their destructor called directly.
	 */
	virtual ~pool() noexcept final;

	/**	\fn allocate() noexcept
	 *	\brief Allocates a free slot.
	 *	\returns Returns a previously free slot or nullptr is there was not one.
	 *	\details Reduces space by one.
	 */
	void *allocate() noexcept;

	/**	\fn discarded(void *addr, bool init) noexcept
	 *	\brief Informs the pool if an allocated slot was uninitialized.
	 *	\param addr The address of the slot.
	 *	\details Unset the bit for the object being allocated if needed.
	 *	\details Deallocate the memory so it can be reused.
	 */
	void discarded(void *addr) noexcept;

	/**	\fn location() const noexcept override final
	 *	\brief Gets an address to compare sources for sorting.
	 *	\returns An address used to compare sources for sorting.
	 */
	virtual void *location() const noexcept override final { return store.location(); }

	/**	\fn from(void *ptr) const noexcept override final
	 *	\brief Checks if a pointer is from the pool.
	 *	\param ptr The object to check.
	 *	\returns slots <= ptr < end
	 */
	virtual bool from(void *ptr) const noexcept override final { return store.from(ptr); }

	/**	\fn base_of(void *ptr) const noexcept override final
	 *	\brief Gets the starting address of the object ptr is within.
	 *	\param ptr The pointer to find the starting address of.
	 *	\returns The originally allocated address containing ptr.
	 *	\pre from(ptr) == true
	 */
	virtual void *base_of(void *ptr) const noexcept override final;

	/**	\fn type_of(void *) const noexcept override final
	 *	\brief Gets the identity of the objects in the pool.
	 *	\returns type.
	 */
	virtual identity const *type_of(void *) const noexcept override final
	{
		return std::addressof(type);
	}

	/**	\fn mark(void *ptr) noexcept override final
	 *	\brief Marks an object as reachable so it will not be deallocate be sweep.
	 *	\param ptr The object to mark as reachable.
	 *	\pre slots <= ptr < end
	 *	\details Sets the appropriate bit in the colors array.
	 */
	virtual void mark(void *ptr) noexcept override final;

	/**	\fn traverse(void *data, enumerate_cb cb) noexcept override final
	 *	\brief Traverses all pending objects from the pool.
	 *	\param data The argument for cb.
	 *	\param cb The callback for enumerating the pointers in traversed objects.
	 *	\returns Returns true if any objects were traversed, otherwise false.
	 */
	virtual bool traverse(void *data, enumerate_cb cb) noexcept override final;

	/**	\fn sweep() noexcept override final
	 *	\brief Deallocates all unmarked objects.
	 *	\details Sets the appropriate bit in the colors array.
	 */
	virtual void sweep() noexcept override final;

	/**	\fn fetch(void *ptr) noexcept override final
	 *	\brief Gets a tracked soft_ptr::data.
	 *	\param ptr The object associated with the soft_ptr::data.
	 *	\returns Returns the associated soft_ptr::data or nullptr if not found.
	 *	\pre from(ptr) == true
	 */
	virtual soft_ptr::data *fetch(void *ptr) noexcept override final;

	/**	\fn used() const noexcept
	 *	\brief Gets the number of allocated slots.
	 *	\returns store.max() - free.available()
	 */
	std::size_t used() const noexcept { return store.max() - free.available(); }

	/**	\fn available() const noexcept
	 *	\brief Gets the number of free slots.
	 *	\returns space
	 */
	std::size_t available() const noexcept { return free.available(); }

	/**	\fn pending() const noexcept
	 *	\brief Gets the number of objects pending traversal.
	 *	\returns gray.pending()
	 */
	std::size_t pending() const noexcept { return gray.pending(); }

	/**	\fn empty() const noexcept
	 *	\brief Checks if the pool is empty.
	 *	\returns store.max() == free.available()
	 */
	bool empty() const noexcept { return store.max() == free.available(); }

	/**	\fn full() const noexcept
	 *	\brief Checks if the pool is full.
	 *	\returns free.full()
	 */
	bool full() const noexcept { return free.full(); }

	/**	\fn has_pending() const noexcept
	 *	\brief Checks if the pool has objects pending traversal.
	 *	\returns gray.has_pending()
	 */
	bool has_pending() const noexcept { return gray.has_pending(); }

	/**	\fn operator==(pool const &other) const noexcept
		 *	\brief Checks if two handles have the same pool.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr == other.ptr.
		 *	\note Only returns true if this and other are the same handle or
		 *	\note both have ptr value of nullptr.
		 */
	constexpr bool operator==(pool const &other) const noexcept { return region == other.region; }

	/**	\fn operator!=(handle const &other) const noexcept
		 *	\brief Checks if two handles are different objects.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr != other.ptr.
		 */
	constexpr bool operator!=(pool const &other) const noexcept { return region != other.region; }

	/**	\fn operator<(handle const &other) const noexcept
		 *	\brief If the owned pool has a lower address that other's.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr < other.ptr.
		 */
	constexpr bool operator<(pool const &other) const noexcept { return region < other.region; }

	/**	\fn operator>(handle const &other) const noexcept
		 *	\brief If the owned pool has a higher address that other's.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr > other.ptr.
		 */
	constexpr bool operator>(pool const &other) const noexcept { return region > other.region; }

	/**	\fn operator<=(handle const &other) const noexcept
		 *	\brief If the owned pool has a lower or equal to the address of other's.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr <= other.ptr.
		 */
	constexpr bool operator<=(pool const &other) const noexcept { return region <= other.region; }

	/**	\fn operator>=(handle const &other) const noexcept
		 *	\brief If the owned pool has a higher or equal to the address of other's.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr >= other.ptr.
		 */
	constexpr bool operator>=(pool const &other) const noexcept { return region >= other.region; }
};

}

#endif
