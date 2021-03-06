
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

/**	\file src/gc/impl/source.hpp
 *	\brief Defines an interface for allocation sources.
 *	\ingroup gc_impl
 */

#include "../callbacks.hpp"
#include "../soft_ptr.hpp"

#ifndef SYNAFIS_GC_SOURCE_HPP
#define SYNAFIS_GC_SOURCE_HPP
#pragma once

namespace gc {

class identity;

/**	\class isource
 *	\brief Represents a source of memory allocation.
 *	\details Used for bookkeeping that needs to know where an object was allocated from.
 *	\ingroup gc_impl
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
	 *	\pre from(ptr) == true
	 */
	virtual void mark(void *ptr) noexcept = 0;

	/**	\fn traverse(void *data, traverse_cb cb) noexcept override final
	 *	\brief Traverses all pending objects from the source.
	 *	\param data The argument for cb.
	 *	\param cb The callback for enumerating the pointers in traversed objects.
	 *	\returns Returns true if any objects were traversed, otherwise false.
	 */
	virtual bool traverse(void *data, enumerate_cb cb) noexcept = 0;

	/**	\fn sweep() noexcept
	 *	\brief Deallocates all unmarked objects.
	 */
	virtual void sweep() noexcept = 0;

	/**	\fn fetch(void *ptr) noexcept
	 *	\brief Gets the soft_ptr::data for ptr or creates it.
	 *	\param ptr The object associated with the soft_ptr::data.
	 *	\returns Returns the associated soft_ptr::data.
	 *	\pre from(ptr) == true
	 */
	virtual soft_ptr::data *fetch(void *ptr) noexcept = 0;
};

}

#endif
