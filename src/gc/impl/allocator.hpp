
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

/**	\file src/gc/impl/allocator.hpp
 *	\brief Defines the type that manages the pools for a type of object.
 *	\ingroup gc_impl
 */

#include "pool.hpp"
#include "core.hpp"

#ifndef SYNAFIS_GC_ALLOCATOR_HPP
#define SYNAFIS_GC_ALLOCATOR_HPP
#pragma once

#include <list>
#include <mutex>

namespace gc {

/**	\class allocator
 *	\brief Type to manage pools for a type of object.
 *	\ingroup gc_impl
 */
class allocator : public collector::iallocator
{
	//!	\cond friends
	friend unit_test::tester<allocator>;
	//!	\endcond

	/**	\fn allocator()
	 *	\brief Deleted.
	 */
	allocator() = delete;

	/**	\fn allocator(allocator const &)
	 *	\brief Deleted.
	 */
	allocator(allocator const &) = delete;

	/**	\fn allocator(allocator &&)
	 *	\brief Deleted.
	 */
	allocator(allocator &&) = delete;

public:
	/**	\typedef pool_list
	 *	\brief The container type for pools.
	 */
	using pool_list = std::list<pool>;

	/**	\typedef pool_iter
	 *	\brief The iterator type of pool_list.
	 */
	using pool_iter = typename pool_list::iterator;

private:
	/**	\var mtx
	 *	\brief The mutex controlling access to the member pools.
	 */
	std::mutex mtx;

	/**	\var current
	 *	\brief The pool to allocate from.
	 */
	pool_iter current;

	/**	\var empty_pools
	 *	\brief A list of pools managed by the allocator.
	 *	\details The pools are partitioned as follows:
	 *	\details - The pool we are currently allocating from.
	 *	\details - The empty pools ordered by address.
	 *	\details - The partially used pools ordered by address.
	 *	\details - The full pools ordered by address.
	 *	\note The first pool is moved to the full partition when it becomes full.
	 *	\invariant All pools have free space and sorted by the address of the pool.
	 *	\todo Investigate if there are move ways to exploit the layout to optimize the allocators.
	 *	\todo Inparticular, if sorting can be done more efficiently that std::list::sort().
	 */
	pool_list empty_pools;

	/**	\var part_pools
	 *	\brief A list of pools managed by the allocator.
	 *	\details The pools are partitioned as follows:
	 *	\details - The pool we are currently allocating from.
	 *	\details - The empty pools ordered by address.
	 *	\details - The partially used pools ordered by address.
	 *	\details - The full pools ordered by address.
	 *	\note The first pool is moved to the full partition when it becomes full.
	 *	\invariant All pools have free space and sorted by the address of the pool.
	 *	\todo Investigate if there are move ways to exploit the layout to optimize the allocators.
	 *	\todo Inparticular, if sorting can be done more efficiently that std::list::sort().
	 */
	pool_list part_pools;

	/**	\var full_pools
	 *	\brief A list of pools with no free slots remaining.
	 *	\invariant All pools are full and sorted by the address of the pool.
	 */
	pool_list full_pools;

	/**	\var type
	 *	\brief The identity of the type the allocator allocates.
	 */
	identity const &type;

	/**	\var cfg
	 *	\brief Stores the pre-calculated information for creating pools.
	 */
	blueprint const cfg;

	/**	\fn try_allocate()
	 *	\brief Attempts to allocate memory for an object.
	 *	\returns Returns a pointer to the allocated memory.
	 *	\throws std::bad_alloc if memory could not be allocated.
	 */
	void *try_allocate();

	/**	\fn allocate_impl()
	 *	\brief Tries to allocate twice with try_allocate.
	 *	\returns Returns a pointer to the allocated memory.
	 *	\details If the first try fails, waits until after a collection cycle to try again.
	 *	\throws std::bad_alloc if memory could not be allocated.
	 */
	void *allocate_impl();

	/**	\fn grow()
	 *	\brief Adds a new pool to the pools list.
	 *	\returns Returns a reference to the new pool.
	 *	\details The new pool will be indicated by current.
	 *	\throws std::bad_alloc if another pool could not be added.
	 *	\pre The mutex mtx must be held by the calling thread.
	 */
	pool &grow();

	/**	\fn move_back() noexcept
	 *	\brief Moves the full pool at the front of pools back.
	 */
	void move_back() noexcept;

public:
	/**	\fn allocator(identity const &id, std::size_t u, traits::flag_type f)
	 *	\brief Constructs a new allocator.
	 *	\param id The identity of the type to allocate for.
	 *	\param u The size of memory to allocate.
	 *	\param f The trait flags for the type.
	 *	\post The pools list will contain one pool.
	 *	\throws std::bad_alloc if memory could not be allocated.
	 */
	allocator(identity const &id, std::size_t u, traits::flag_type f);

	/**	\fn ~allocator() noexcept final
	 *	\brief Destructor.
	 */
	virtual ~allocator() noexcept final;

	/**	\fn allocate() override final
	 *	\brief Allocates memory for an object.
	 *	\returns Returns a pointer to the allocated memory.
	 *	\throws std::bad_alloc if memory could not be allocated.
	 */
	virtual void *allocate() override final;

	/**	\fn allocate(std::nothrow_t) noexcept override final
	 *	\brief Allocates memory for an object.
	 *	\returns Returns a pointer to the allocated memory or nullptr on failure.
	 */
	virtual void *allocate(std::nothrow_t) noexcept override final;

	/**	\fn discarded(void *addr) noexcept override final
	 *	\brief Informs the allocator that an allocated object was not initialized.
	 *	\param addr The address of the previously allocated memory.
	 *	\details If the collector lock was properly held since allocate() was called, then
	 *	\details either the object is from the front of pools or was moved to the full range.
	 */
	virtual void discarded(void *addr) noexcept override final;

	/**	\fn shrink(std::size_t goal) noexcept override final
	 *	\brief Causes the allocator to try and free unneeded memory.
	 *	\param goal A hint about how much memory to try and free.
	 *	\returns Returns the units of memory freed.
	 *	\details The measures of memory are not exact, just hints. Assume a pool is one unit.
	 *	\details Regardless of the goal, free pools such that free space only exceeds the used
	 *	\details space by one pool's capacity after rounding down to the nearest multiple of
	 *	\details capacity.
	 *	\note Will keep at least on pool's capacity of free space regardless of goal.
	 */
	virtual std::size_t shrink(std::size_t goal) noexcept override final;
};

}

#endif
