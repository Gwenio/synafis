
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

#ifndef SYNAFIS_GC_ALLOCATOR_HPP
#define SYNAFIS_GC_ALLOCATOR_HPP
#pragma once

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

#ifndef SYNAFIS_GC_POOL_HPP
#include "pool.hpp"
#endif

#ifndef SYNAFIS_GC_IMPL_HPP
#include "impl.hpp"
#endif

/**	\file src/gc/allocator.hpp
 *	\brief Defines the type that manages the pools for a type of object.
 */

#include <list>
#include <mutex>

namespace gc {

/**	\class allocator
 *	\brief Type to manage pools for a type of object.
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

private:
	/**	\typedef handle
	 *	\brief Shorthand for pool::handle.
	 */
	typedef pool::handle handle;

	/**	\var mtx
	 *	\brief The mutex controlling access to the member pools.
	 */
	std::mutex mtx;

	/**	\var pools
	 *	\brief A list of pools managed by the allocator.
	 *	\details The pools are partitioned as follows:
	 *	\details - The pool we are currently allocating from.
	 *	\details - The empty pools ordered by address.
	 *	\details - The partially used pools ordered by address.
	 *	\details - The full pools ordered by address.
	 *	\note The first pool is moved to the full partition when it becomes full.
	 *	\todo Investigate if there are move ways to exploit the layout to optimize the allocators.
	 *	\todo Inparticular, if sorting can be done more efficiently that std::list::sort().
	 */
	std::list<handle> pools;

	/**	\var full_begin
	 *	\brief The iterator position to the first full pool is pools.
	 */
	std::list<handle>::const_iterator full_begin;

	/**	\var type
	 *	\brief The identity of the type the allocator allocates.
	 */
	identity const &type;

	/**	\var unit
	 *	\brief The size of a unit of allocated memory.
	 *	\details The size of an object rounded up to a multiple of its alignment.
	 */
	std::size_t const unit;

	/**	\var capacity
	 *	\brief The capacity of pools in the allocator.
	 *	\details Determined by pool::select_capacity.
	 */
	std::size_t const capacity;

	/**	\var flags
	 *	\brief Stores the trait flags of the type.
	 */
	traits::flag_type const flags;

	/**	\fn allocate_impl()
	 *	\brief Implements the bulk of the public allocate() methods.
	 *	\returns Returns a pointer to the allocated memory or nullptr on failure.
	 *	\throws std::bad_alloc if memory could not be allocated.
	 */
	void *allocate_impl();

	/**	\fn grow()
	 *	\brief Adds a new pool to the pools list.
	 *	\returns Returns a reference to the new pool's handle.
	 *	\throws std::bad_alloc if another pool could not be added.
	 *	\pre The mutex mtx must be held by the calling thread.
	 */
	handle &grow();

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
