
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

#ifndef SYNAFIS_GC_IDACCESS_HPP
#define SYNAFIS_GC_IDACCESS_HPP
#pragma once

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

/**	\file gc/idaccess.hpp
 *	\brief Implements identity::idaccess.
 *	\warning Should only be included where the more access to identity
 *	\warning is appropriate.
 */

namespace gc {

class identity::idaccess {
	/**	\fn idaccess()
	 *	\brief Deleted.
	 */
	idaccess() = delete;

	/**	\fn ~idaccess()
	 *	\brief Deleted.
	 */
	~idaccess() = delete;
public:
	/**	\fn finalize(identity const& id, void *ptr) noexcept
	 *	\brief Wrapper for identity::finalize that provides access to it.
	 *	\param id The identity of the object.
	 *	\param ptr A pointer to the object to clean up.
	 */
	static void finalize(identity const& id, void *ptr) noexcept {
		id.finalize(ptr);
	}

	/**	\fn traverse(identity const& id, void const* ptr, void *data, enumerate_cb cb) noexcept
	 *	\brief Wrapper for identity::traverse that provides access to it.
	 *	\param id The identity of the object.
	 *	\param ptr A pointer to the object to traverse.
	 *	\param data Parameter to pass to cb.
	 *	\param cb Callback to enumerate reachable pointers.
	 */
	static void traverse(identity const& id, void const* ptr, void *data, enumerate_cb cb) noexcept {
		id.traverse(ptr, data, cb);
	}

	/**	\fn relocate(identity const& id, void *orig, void *dest, void *data, remap_cb cb)
	 *	\brief Wrapper for identity::relocate that provides access to it.
	 *	\param id The identity of the object.
	 *	\param orig The original address of the object.
	 *	\param orig The destination address of the object.
	 *	\param data Parameter to pass to cb.
	 *	\param cb Callback to remap pointers.
	 */
	static void relocate(identity const& id, void *orig, void *dest, void *data, remap_cb cb) noexcept {
		id.relocate(orig, dest, data, cb);
	}
};

}

#endif
