
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

#ifndef SYNAFIS_GC_SOURCE_HPP
#define SYNAFIS_GC_SOURCE_HPP
#pragma once

namespace gc {

/**	\class isource
 *	\brief Represents a source of memory allocation.
 *	\details Used for bookkeeping that needs to know where an object was allocated from.
 */
class isource
{
protected:
	/**	\fn ~isource()
	 *	\brief Default constructor.
	 */
	constexpr isource() noexcept = default;

public:
	/**	\fn ~isource()
	 *	\brief Default destructor.
	 */
	virtual ~isource() noexcept = default;

	/**	\fn location() const noexcept
	 *	\brief Gets an address to compare sources for sorting.
	 *	\returns An address used to compare sources for sorting.
	 */
	virtual void *location() const noexcept = 0;

	/**	\fn from(void *ptr) const noexcept
	 *	\brief Checks if a pointer is from the pool.
	 *	\param ptr The object to check.
	 *	\returns slots <= ptr < end
	 */
	virtual bool from(void *ptr) const noexcept = 0;

	/**	\fn base_of(void *ptr) const noexcept
	 *	\brief Gets the starting address of the object ptr is within.
	 *	\param ptr The pointer to find the starting address of.
	 *	\returns The originally allocated address containing ptr.
	 *	\pre from(ptr) == true
	 */
	virtual void *base_of(void *ptr) const noexcept = 0;

	/**	\fn type_of(void *ptr) const noexcept
	 *	\brief Gets the identity of ptr.
	 *	\param ptr The pointer to find the type of.
	 *	\returns The identity of ptr.
	 *	\pre from(ptr) == true
	 */
	virtual identity const *type_of(void *ptr) const noexcept = 0;

	/**	\fn mark(void *ptr) noexcept
	 *	\brief Marks an object as reachable so it will not be deallocate be sweep.
	 *	\param ptr The object to mark as reachable.
	 */
	virtual void mark(void *ptr) noexcept = 0;

	/**	\fn sweep() noexcept
	 *	\brief Deallocates all unmarked objects.
	 */
	virtual void sweep() noexcept = 0;
};

}

#endif
