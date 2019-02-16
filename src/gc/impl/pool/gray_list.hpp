
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

/**	\file src/gc/impl/pool/gray_list.hpp
 *	\brief Defines the type tracking 'gray' slots for gc::pool.
 *	\ingroup gc_pool
 */

#include "../../../unit_test.hpp"

#ifndef SYNAFIS_GC_POOL_GRAY_LIST_HPP
#define SYNAFIS_GC_POOL_GRAY_LIST_HPP
#pragma once

namespace gc {

class pool;

/**	\class gray_list
 *	\brief The list of free slots for a pool.
 *	\ingroup gc_pool
 */
class gray_list
{
	//!	\cond friends
	friend unit_test::tester<gray_list>;
	friend unit_test::tester<pool>;
	//!	\endcond

	/**	\fn gray_list()
	 *	\brief Deleted.
	 */
	gray_list() = delete;

	/**	\fn gray_list(gray_list const &)
	 *	\brief Deleted.
	 */
	gray_list(gray_list const &) = delete;

	/**	\fn gray_list(gray_list &&)
	 *	\brief Deleted.
	 */
	gray_list(gray_list &&) = delete;

	/**	\fn operator=(gray_list const &)
	 *	\brief Deleted.
	 */
	gray_list &operator=(gray_list const &) = delete;

	/**	\fn operator=(gray_list &&)
	 *	\brief Deleted.
	 */
	gray_list &operator=(gray_list &&) = delete;

private:
	/**	\var sentinel
	 *	\brief Points to the bottom of the gray stack.
	 */
	void **const sentinel;

	/**	\var current
	 *	\brief Points to the position beyond the top of the gray stack.
	 *	\invariant current >= sentinel
	 */
	void **current;

public:
	/**	\fn gray_list(void **begin) noexcept
	 *	\brief Constructor.
	 *	\param begin The beginning of the gray_list.
	 */
	gray_list(void **begin) noexcept : sentinel(begin), current(begin) {}

	/**	\fn ~gray_list() noexcept
	 *	\brief Default destructor.
	 */
	~gray_list() noexcept = default;

	/**	operator bool() const noexcept
	 *	\brief Checks that there is an actual gray list.
	 *	\returns sentinel != nullptr
	 */
	operator bool() const noexcept { return sentinel != nullptr; }

	/**	\fn pop() noexcept
	 *	\brief Get a gray slot to traverse and remove it from the list.
	 *	\returns Returns the address of a gray slot.
	 *	\pre has_pending()
	 */
	void *pop() noexcept
	{
		SYNAFIS_ASSERT(has_pending());
		current--;
		return *current;
	}

	/**	\fn push(void *slot) noexcept
	 *	\brief Adds a slot to the list.
	 *	\param slot The slot to push into the gray_list.
	 */
	void push(void *slot) noexcept
	{
		*current = slot;
		current++;
	}

	/**	\fn pending() const noexcept
	 *	\brief Gets the number of objects pending traversal.
	 *	\returns current - sentinel
	 */
	std::size_t pending() const noexcept { return static_cast<std::size_t>(current - sentinel); }

	/**	\fn has_pending() const noexcept
	 *	\brief Checks if the pool has objects pending traversal.
	 *	\returns sentinel < current
	 */
	bool has_pending() const noexcept { return sentinel < current; }
};

}

#endif
