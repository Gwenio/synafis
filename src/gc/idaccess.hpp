
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

#ifndef SYNAFIS_GC_IDACCESS_HPP
#define SYNAFIS_GC_IDACCESS_HPP
#pragma once

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

/**	\file src/gc/idaccess.hpp
 *	\brief Implements identity::access.
 *	\warning Should only be included where the more access to identity
 *	\warning is appropriate.
 */

namespace gc {

class identity::access
{
	/**	\fn access()
	 *	\brief Deleted.
	 */
	access() = delete;

	/**	\fn ~access()
	 *	\brief Deleted.
	 */
	~access() = delete;

public:
	/**	\fn finalize(identity const &id, void *ptr) noexcept
	 *	\brief Wrapper for identity::finalize() that provides access to it.
	 *	\param id The identity of the object.
	 *	\param ptr A pointer to the object to clean up.
	 */
	static void finalize(identity const &id, void *ptr) noexcept { id.finalize(ptr); }

	/**	\fn has_finalizer(identity const &id) noexcept
	 *	\brief Checks if id has a finalizer callback.
	 *	\param id The identity of the object type.
	 *	\returns Returns true if id has a finalizer callback.
	 */
	static bool has_finalizer(identity const &id) noexcept { return id.fcb != nullptr; }

	/**	\fn traverse(identity const &id, void const *ptr, void *data, enumerate_cb cb) noexcept
	 *	\brief Wrapper for identity::traverse() that provides access to it.
	 *	\param id The identity of the object.
	 *	\param ptr A pointer to the object to traverse.
	 *	\param data Parameter to pass to cb.
	 *	\param cb Callback to enumerate reachable pointers.
	 */
	static void traverse(identity const &id, void const *ptr, void *data, enumerate_cb cb) noexcept
	{
		id.traverse(ptr, data, cb);
	}

	/**	\fn has_traverser(identity const &id) noexcept
	 *	\brief Checks if id has a traversal callback.
	 *	\param id The identity of the object type.
	 *	\returns Returns true if id has a traversal callback.
	 */
	static bool has_traverser(identity const &id) noexcept { return id.tcb != nullptr; }

	/**	\fn relocate(identity const &id, void *orig, void *dest, void *data, remap_cb cb)
	 *	\brief Wrapper for identity::relocate() that provides access to it.
	 *	\param id The identity of the object.
	 *	\param orig The original address of the object.
	 *	\param dest The destination address of the object.
	 *	\param data Parameter to pass to cb.
	 *	\param cb Callback to remap pointers.
	 */
	static void relocate(
		identity const &id, void *orig, void *dest, void *data, remap_cb cb) noexcept
	{
		id.relocate(orig, dest, data, cb);
	}

	/**	\fn unit_size() noexcept
	 *	\brief Gets the amount of memory to allocate to objects of a type.
	 *	\tparam T The type to get the allocation unit for.
	 *	\returns Returns the size of the allocation unit for objects of type T.
	 *	\see identity::unit_size()
	 */
	template<typename T>
	static constexpr std::size_t unit_size() noexcept
	{
		return identity::unit_size<T>();
	}
};

/**	\typedef idaccess
 *	\brief Shorthand for identity::access.
 */
typedef identity::access idaccess;

}

#endif
