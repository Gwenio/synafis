
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

/**	\file src/gc/impl/pool/free_list.hpp
 *	\brief Defines type implementing the free slot list for gc::pool.
 *	\ingroup gc_pool
 */

#include "../../../unit_test.hpp"
#include "arena.hpp"

#ifndef SYNAFIS_GC_POOL_FREE_LIST_HPP
#define SYNAFIS_GC_POOL_FREE_LIST_HPP
#pragma once

namespace gc {

class pool;

/**	\class free_list
 *	\brief The list of free slots for a pool.
 *	\ingroup gc_pool
 */
class free_list
{
	//!	\cond friends
	friend unit_test::tester<free_list>;
	friend unit_test::tester<pool>;
	//!	\endcond

	/**	\fn free_list()
	 *	\brief Deleted.
	 */
	free_list() = delete;

	/**	\fn free_list(free_list const &)
	 *	\brief Deleted.
	 */
	free_list(free_list const &) = delete;

	/**	\fn free_list(free_list &&)
	 *	\brief Deleted.
	 */
	free_list(free_list &&) = delete;

	/**	\fn operator=(free_list const &)
	 *	\brief Deleted.
	 */
	free_list &operator=(free_list const &) = delete;

	/**	\fn operator=(free_list &&)
	 *	\brief Deleted.
	 */
	free_list &operator=(free_list &&) = delete;

private:
	/**	\class node
	 *	\brief Used to form a stack of free locations.
	 *	\details A node is located at the start of the corresponding free locations;
	 *	\details thus, the location can be retrieve casting a pointer to the node to
	 *	\details a void pointer.
	 *	\details
	 *	\details This method of tracking free objects makes all allocations and
	 *	\details deallocations take O(1) time.
	 *	\details This does not mean it is the most efficient; rather, it means
	 *	\details times are reliable.
	 */
	struct node
	{
	public:
		/**	\var next
		 *	\brief The next free location.
		 */
		node *next;
	};

	/**	\var head
	 *	\brief The next free object to allocate. Acts as a stack.
	 *	\note If head == nullptr, there is no free memory to allocate.
	 */
	node *head;

	/**	\var space
	 *	\brief The number of free slots.
	 *	\invariant If and only if head == nullptr then space == 0.
	 */
	std::size_t space;

public:
	/**	\fn free_list(arena const &store) noexcept
	 *	\brief Constructor.
	 *	\param store The arena the free_list tracks free slots in.
	 */
	free_list(arena const &store) noexcept;

	/**	\fn ~free_list() noexcept
	 *	\brief Default destructor.
	 */
	~free_list() noexcept = default;

	/**	\fn pop() noexcept
	 *	\brief Removes the next free slot and returns it.
	 *	\returns Returns the address of a free slot.
	 *	\details Reduces space by one.
	 *	\pre !full()
	 */
	void *pop() noexcept;

	/**	\fn push(void *slot) noexcept
	 *	\brief Preforms bookkeeping related to freeing a slot.
	 *	\param slot The slot to push into the free_list.
	 *	\details The address becomes the top of the free stack, storing the old top
	 *	\details as the next node.
	 *	\details
	 *	\details Increments space.
	 */
	void push(void *slot) noexcept;

	/**	\fn available() const noexcept
	 *	\brief Gets the number of free slots.
	 *	\returns space
	 */
	std::size_t available() const noexcept { return space; }

	/**	\fn full() const noexcept
	 *	\brief Checks if the pool is full.
	 *	\returns space == 0
	 */
	bool full() const noexcept { return space == 0; }

	/**	\fn node_size() noexcept
	 *	\brief The minimum unit size for pool allocations.
	 */
	static constexpr std::size_t node_size() noexcept
	{
		static_assert(sizeof(node *) % alignof(node *) == 0,
			"sizeof(node *) must be a multiple of alignof(node *).");
		return sizeof(node *);
	}
};

}

#endif
