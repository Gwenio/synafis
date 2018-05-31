
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

#if !defined(SYNAFIS_DATATYPES_HPP)
#include "../datatypes.hpp"
#elif !defined(SYNAFIS_DATATYPES_ENVIRONMENT_TYPE_HPP)
#define SYNAFIS_DATATYPES_ENVIRONMENT_TYPE_HPP
#pragma once

/**	\file datatypes/environment_type.hpp
 *	\brief Defines the type for key-value bindings for symbol lookup.
 */

namespace datatype {

/**	\class environment_type
 *	\brief The type for storing key-value bindings.
 *	\todo Currently just a placeholder.
 */
class environment_type {
public:
	/**	\fn environment_type()
	 *	\brief Default.
	 */
	environment_type() = default;

	/**	\fn ~environment_type()
	 *	\brief Default.
	 */
	~environment_type() = default;

	template<typename Key>
	value_type operator[](Key key) const;
	template<typename Keys>
	void bind(Keys keys, value_type value);

	/**	\fn traverse(void *arg, gc::enumerate_cb cb) const noexcept
	 *	\brief Traverses pointers for the garbage collector.
	 *	\param arg Optional parameter to pass to cb.
	 *	\param cb The callback to enumerate pointers.
	 */
	void traverse(void *arg, gc::enumerate_cb cb) const noexcept;

	/**	\fn remap(void *arg, gc::remap_cb cb) noexcept
	 *	\brief Remaps pointers for the garbage collector.
	 *	\param arg Optional parameter to pass to cb.
	 *	\param cb The callback to remap pointers.
	 */
	void remap(void *arg, gc::remap_cb cb) noexcept;
};

}

namespace gc {

namespace traits {

template<>
class pointers_type<datatype::environment_type> :
	public std::integral_constant<bool, true> {};

}

}

#endif
