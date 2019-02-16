
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
	typedef std::bitset<(sizeof(std::uintptr_t) * 8)> bit_group;

	/**	\class handle
	 *	\brief Manages the ownership and lifetime of a pool.
	 *	\ingroup gc_pool
	 */
	class handle
	{
		//!	\cond friends
		// Shares a tester with pool.
		friend unit_test::tester<pool>;
		//!	\endcond
	private:
		/** \var ptr
		 *	\brief The pointer to the managed pool object.
		 */
		pool *ptr;

		/**	\fn destroy() noexcept
		 *	\brief Destroys the owned pool.
		 *	\pre ptr != nullptr
		 */
		void destroy() noexcept { ptr->~pool(); }

	public:
		/**	\fn handle() noexcept
		 *	\brief Initialize with a null pointer.
		 */
		constexpr handle() noexcept : ptr(nullptr) {}

		/**	\fn handle(std::nullptr_t) noexcept
		 *	\brief Initialize with a null pointer.
		 */
		constexpr handle(std::nullptr_t) noexcept : handle() {}

		/**	\fn handle(handle const &)
		 *	\brief Deleted.
		 */
		handle(handle const &) = delete;

		/**	\fn handle(handle &&other)
		 *	\brief Moves the pointer of other to this.
		 *	\param other The handle to take ownership from.
		 */
		handle(handle &&other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}

		/**	\fn handle(identity const &id, std::size_t capacity, std::size_t unit) noexcept
		 *	\brief Creates a new pool.
		 *	\param id The type the pool allocates memory for.
		 *	\param capacity The number of objects in the pool.
		 *	\param unit The size of the unit of allocation.
		 *	\throws std::bad_alloc if virtual memory could not be allocated.
		 *	\pre Must be at least min unit.
		 *	\note To deallocate the pool, directly call its destructor.
		 *	\see gc::pool::select_capacity
		 */
		handle(identity const &id, std::size_t capacity, std::size_t unit);

		/**	\fn ~handle() noexcept
		 *	\brief Destroys ptr if it is not null.
		 */
		~handle() noexcept
		{
			if (ptr) { destroy(); }
		}

		/**	\fn handle &operator=(std::nullptr_t) noexcept
		 *	\brief Initialize with a null pointer.
		 */
		handle &operator=(std::nullptr_t) noexcept
		{
			if (ptr) {
				destroy();
				ptr = nullptr;
			}
			return *this;
		}

		/**	\fn operator=(handle const &)
		 *	\brief Deleted.
		 */
		handle &operator=(handle const &) = delete;

		/**	\fn operator=(handle &&other) noexcept
		 *	\brief Moves the pointer of other to this.
		 *	\param other The handle to take ownership from.
		 */
		handle &operator=(handle &&other) noexcept
		{
			if (*this != other) {
				if (ptr) { destroy(); }
				ptr = std::exchange(other.ptr, nullptr);
			}
			return *this;
		}

		/**	\fn operator!() const noexcept
		 *	\brief Checks that ptr is null.
		 *	\returns Returns true if ptr is null.
		 */
		constexpr bool operator!() const noexcept { return ptr == nullptr; }

		/**	\fn operator bool() const noexcept
		 *	\brief Checks that ptr is not null.
		 *	\returns Returns true if ptr is not null.
		 */
		constexpr operator bool() const noexcept { return ptr != nullptr; }

		/**	\fn operator isource &() noexcept
		 *	\brief Gets a reference to the owned pool.
		 *	\returns *ptr
		 *	\pre ptr != nullptr
		 */
		constexpr operator isource &() noexcept { return *ptr; }

		/**	\fn operator isource const &() const noexcept
		 *	\brief Gets a reference to the owned pool.
		 *	\returns *ptr
		 *	\pre ptr != nullptr
		 */
		constexpr operator isource const &() const noexcept { return *ptr; }

		/**	\fn operator*() noexcept
		 *	\brief Gets a reference to the owned pool.
		 *	\returns *ptr
		 *	\pre ptr != nullptr
		 */
		constexpr pool &operator*() noexcept { return *ptr; }

		/**	\fn operator*() const noexcept
		 *	\brief Gets a reference to the owned pool.
		 *	\returns *ptr
		 *	\pre ptr != nullptr
		 */
		constexpr pool const &operator*() const noexcept { return *ptr; }

		/**	\fn operator==(handle const &other) const noexcept
		 *	\brief Checks if two handles have the same pool.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr == other.ptr.
		 *	\note Only returns true if this and other are the same handle or
		 *	\note both have ptr value of nullptr.
		 */
		constexpr bool operator==(handle const &other) const noexcept { return ptr == other.ptr; }

		/**	\fn operator!=(handle const &other) const noexcept
		 *	\brief Checks if two handles are different objects.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr != other.ptr.
		 */
		constexpr bool operator!=(handle const &other) const noexcept { return ptr != other.ptr; }

		/**	\fn operator<(handle const &other) const noexcept
		 *	\brief If the owned pool has a lower address that other's.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr < other.ptr.
		 */
		constexpr bool operator<(handle const &other) const noexcept { return ptr < other.ptr; }

		/**	\fn operator>(handle const &other) const noexcept
		 *	\brief If the owned pool has a higher address that other's.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr > other.ptr.
		 */
		constexpr bool operator>(handle const &other) const noexcept { return ptr > other.ptr; }

		/**	\fn operator<=(handle const &other) const noexcept
		 *	\brief If the owned pool has a lower or equal to the address of other's.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr <= other.ptr.
		 */
		constexpr bool operator<=(handle const &other) const noexcept { return ptr <= other.ptr; }

		/**	\fn operator>=(handle const &other) const noexcept
		 *	\brief If the owned pool has a higher or equal to the address of other's.
		 *	\param other The other handle to compare with.
		 *	\returns Returns true if ptr >= other.ptr.
		 */
		constexpr bool operator>=(handle const &other) const noexcept { return ptr >= other.ptr; }

		/**	\fn operator==(std::nullptr_t) const noexcept
		 *	\brief Checks that ptr is null.
		 *	\returns Returns true if ptr is null.
		 */
		constexpr bool operator==(std::nullptr_t) const noexcept { return ptr == nullptr; }

		/**	\fn operator!=(std::nullptr_t) const noexcept
		 *	\brief Checks that ptr is not null.
		 *	\returns Returns true if ptr is not null.
		 */
		constexpr bool operator!=(std::nullptr_t) const noexcept { return ptr != nullptr; }

		/**	\fn allocate() noexcept
		 *	\brief Allocates a free slot.
		 *	\returns Returns ptr->allocate().
		 *	\pre ptr != nullptr
		 *	\see pool::allocate
		 */
		void *allocate() noexcept { return ptr->allocate(); }

		/**	\fn discarded(void *addr) noexcept
		 *	\brief Informs the pool if an allocated slot was uninitialized.
		 *	\param addr The address of the slot.
		 *	\pre ptr != nullptr
		 *	\see pool::discarded
		 */
		void discarded(void *addr) noexcept
		{
			SYNAFIS_ASSERT(ptr != nullptr);
			ptr->discarded(addr);
		}

		/**	\fn mark(void *addr) noexcept
		 *	\brief Marks an object as reachable so it will not be deallocate be sweep.
		 *	\param addr The object to mark as reachable.
		 *	\details Calls ptr->mark(addr).
		 *	\pre ptr != nullptr
		 *	\see pool::mark
		 */
		void mark(void *addr) noexcept { ptr->mark(addr); }

		/**	\fn from(void *addr) const noexcept
		 *	\brief Checks if a pointer is from the pool.
		 *	\param addr The object to check.
		 *	\returns ptr->from(addr)
		 *	\pre ptr != nullptr
		 *	\see pool::from
		 */
		bool from(void *addr) const noexcept { return ptr->from(addr); }

		/**	\fn sweep() noexcept
		 *	\brief Deallocates all unmarked objects.
		 *	\details Calles ptr->sweep().
		 *	\pre ptr != nullptr
		 *	\see pool::sweep
		 */
		void sweep() noexcept { ptr->sweep(); }

		/**	\fn used() const noexcept
		 *	\brief Gets the number of allocated slots.
		 *	\returns ptr->used().
		 *	\pre ptr != nullptr
		 *	\see pool::used
		 */
		std::size_t used() const noexcept { return ptr->used(); }

		/**	\fn available() const noexcept
		 *	\brief Gets the number of free slots.
		 *	\returns ptr->available().
		 *	\pre ptr != nullptr
		 *	\see pool::available
		 */
		std::size_t available() const noexcept { return ptr->available(); }

		/**	\fn pending() const noexcept
		 *	\brief Gets the number of objects pending traversal.
		 *	\returns ptr->pending()
		 *	\pre ptr != nullptr
		 *	\see pool::pending
		 */
		std::size_t pending() const noexcept { return ptr->pending(); }

		/**	\fn empty() const noexcept
		 *	\brief Checks if the pool is empty.
		 *	\returns ptr->empty()
		 *	\pre ptr != nullptr
		 *	\see pool::empty
		 */
		bool empty() const noexcept { return ptr->empty(); }

		/**	\fn full() const noexcept
		 *	\brief Checks if the pool is full.
		 *	\returns ptr->full()
		 *	\pre ptr != nullptr
		 *	\see pool::full
		 */
		bool full() const noexcept { return ptr->full(); }

		/**	\fn has_pending() const noexcept
		 *	\brief Checks if the pool has objects pending traversal.
		 *	\returns ptr->has_pending()
		 *	\pre ptr != nullptr
		 *	\see pool::has_pending
		 */
		bool has_pending() const noexcept { return ptr->has_pending(); }
	};

	//!	\cond friends
	friend handle;
	//!	\endcond
private:
	/**	\var region
	 *	\brief The memory region of the pool.
	 *	\note The intent is for the pool to be allocated in the block of region.
	 */
	vmem region;

	/**	\var capacity
	 *	\brief The number of slots in the pool.
	 */
	std::size_t const capacity;

	/**	\var unit
	 *	\brief The allocation unit size.
	 *	\note At least the sizeof the type being allocated and a multiple of its alignof.
	 */
	std::uintptr_t const unit;

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

	/**	\var bitmap
	 *	\brief Tracks allocated slots. Is a pointer to an array.
	 *	\details While nodes track free objects for allocation, the bitmap is
	 *	\details needed to know if the finalizer should be called on a slot
	 *	\details that is not marked.
	 *	\todo The bitmap is not needed for types without a finalizer.
	 *	\note Could be done with a node list similar to free objects;
	 *	\note however, if the pointer cannot be squeezed in after the
	 *	\note object then it would take more memory than a bitmap.
	 */
	bit_group *bitmap;

	/**	\var colors
	 *	\brief Tracks reachable objects. Is a pointer to an array.
	 *	\note Slots with their matching bit set are called 'black' while
	 *	\note the others are 'white'.
	 */
	bit_group *colors;

	/**	\var sentinel
	 *	\brief Points to the bottom of the gray stack.
	 */
	void **const sentinel;

	/**	\var gray
	 *	\brief Points to the position beyond the top of the gray stack.
	 */
	void **gray;

	/**	\var slots
	 *	\brief The start of the address range to allocate objects from.
	 */
	void *const slots;

	/**	\var end
	 *	\brief The end of the address range to allocate objects from.
	 *	\invariant Equal to slots + (unit * \<number of slots in the pool\>).
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

	/**	\fn bit_locate(void *ptr) const noexcept
	 *	\brief Calculates the information for accessing the bit associated with an address.
	 *	\param ptr The slot to calculate fore.
	 *	\returns Returns a tuple containing the offset for the bit group and the bit number.
	 */
	std::tuple<std::size_t, std::size_t> bit_locate(void *ptr) const noexcept;

	/**	\fn pool(vmem &&mem, identity const &id, std::size_t cap, std::size_t u, void **g, void *start) noexcept
	 *	\brief Called by create to initialize a new pool.
	 *	\param mem The virtual memory the pool is allocated on is to be owned by the pool.
	 *	\param id The type of object placed in the pool.
	 *	\param cap The maximum capacity of the pool.
	 *	\param u The unit size of slots in the pool.
	 *	\param g The start of the gray stack and the value of sentinel.
	 *	\param start The starting address for the memory to allocate objects from.
	 *	\details Sets the bits in bitmap and colors to false.
	 *	\details Places all slots in the free stack.
	 *	\pre The value of 'g' must be nullptr if the type is not traversable.
	 *	\pre The value of 'g' must be a valid pointer if the type is traversable.
	 */
	pool(vmem &&mem, identity const &id, std::size_t cap, std::size_t u, void **g,
		void *start) noexcept;

public:
	/**	\var min_unit
	 *	\brief The minimum unit size for pool allocations.
	 */
	static constexpr std::size_t const min_unit{free_list::node_size()};

	/**	\fn ~pool() noexcept final
	 *	\brief Finalizes all objects and frees region.
	 *	\note Pools are to have their destructor called directly.
	 */
	virtual ~pool() noexcept final;

	/**	\fn select_capacity(std::size_t unit) noexcept
	 *	\brief Determines a good capacity for a pool with a given allocation unit.
	 *	\param unit The size of the unit of allocation.
	 *	\returns Returns the selected capacity.
	 *	\pre Must be at least min unit.
	 *	\todo Currently does not consider if the bitmap and colors array make
	 *	\todo inefficient use of the last page they occupy.
	 *	\todo However, that consideration only applies if the arrays can pass
	 *	\todo the end of the page they start on.
	 *	\see config::min_pool
	 *	\see config::max_pool
	 */
	static std::size_t select_capacity(std::size_t unit) noexcept;

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
	virtual void *location() const noexcept override final { return slots; }

	/**	\fn from(void *ptr) const noexcept override final
	 *	\brief Checks if a pointer is from the pool.
	 *	\param ptr The object to check.
	 *	\returns slots <= ptr < end
	 */
	virtual bool from(void *ptr) const noexcept override final { return slots <= ptr && ptr < end; }

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
	 *	\returns capacity - free.available()
	 */
	std::size_t used() const noexcept { return capacity - free.available(); }

	/**	\fn available() const noexcept
	 *	\brief Gets the number of free slots.
	 *	\returns space
	 */
	std::size_t available() const noexcept { return free.available(); }

	/**	\fn pending() const noexcept
	 *	\brief Gets the number of objects pending traversal.
	 *	\returns gray - sentinel
	 */
	std::size_t pending() const noexcept { return static_cast<std::size_t>(gray - sentinel); }

	/**	\fn empty() const noexcept
	 *	\brief Checks if the pool is empty.
	 *	\returns capacity == free.available()
	 */
	bool empty() const noexcept { return capacity == free.available(); }

	/**	\fn full() const noexcept
	 *	\brief Checks if the pool is full.
	 *	\returns free.full()
	 */
	bool full() const noexcept { return free.full(); }

	/**	\fn has_pending() const noexcept
	 *	\brief Checks if the pool has objects pending traversal.
	 *	\returns sentinel < gray
	 */
	bool has_pending() const noexcept { return sentinel < gray; }
};

}

#endif
