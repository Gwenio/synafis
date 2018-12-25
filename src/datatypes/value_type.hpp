
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
#elif !defined(SYNAFIS_DATATYPES_VALUE_TYPE_HPP)
#define SYNAFIS_DATATYPES_VALUE_TYPE_HPP
#pragma once

/**	\file src/datatypes/value_type.hpp
 *	\brief Defines the type for storing type erased values.
 */

namespace datatype {

/**	\class value_type
 *	\brief The type erasing container for other data types.
 *	\todo Document once the interface is more stable.
 */
class value_type {
	friend unit_test::tester<value_type>;
private:
	/**	\var store
	 *	\brief Stores the value.
	 */
	std::variant<empty_type, inert_type, ignore_type, bool,
		block, gc::hard_ptr, gc::soft_ptr> store;
public:
	/**	\fn value_type() noexcept
	 *	\brief Initializes to contain empty_type.
	 *	\todo Determine if inert or ignore would be a better default,
	 *	\todo or maybe the default constructor should be deleted.
	 */
	constexpr value_type() noexcept : store(empty::value) {}

	value_type(value_type const& other) noexcept :
		store(other.store) {}
	value_type(value_type && other) noexcept :
		store(std::move(other.store)) {}
	constexpr value_type(empty_type) noexcept : store(empty::value) {}
	constexpr value_type(inert_type) noexcept : store(inert::value) {}
	constexpr value_type(ignore_type) noexcept : store(ignore::value) {}
	constexpr value_type(bool b) noexcept : store(b) {}
	constexpr value_type(block b) noexcept : store(b) {}
	constexpr value_type(gc::hard_ptr const& p) noexcept : store(p) {}
	constexpr value_type(gc::hard_ptr && p) noexcept :
		store(std::forward<gc::hard_ptr>(p)) {}
	constexpr value_type(gc::soft_ptr const& p) noexcept : store(p) {}
	constexpr value_type(gc::soft_ptr && p) noexcept :
		store(std::forward<gc::soft_ptr>(p)) {}

	/**	\fn ~value_type()
	 *	\brief Default.
	 */
	~value_type() = default;

	operator bool() const {
		return std::get<bool>(store);
	}
	operator block() const {
		return std::get<block>(store);
	}
	operator gc::soft_ptr() const {
		return std::get<gc::soft_ptr>(store);
	}
	value_type &operator=(bool value) noexcept {
		store = value;
		return *this;
	}
	value_type &operator=(block value) noexcept {
		store = value;
		return *this;
	}
	value_type &operator=(gc::soft_ptr value) {
		store = std::move(value);
		return *this;
	}
	value_type &operator=(value_type const& other) noexcept {
		if (this != std::addressof(other)) {
			store = other.store;
		}
		return *this;
	}
	value_type &operator=(empty_type const& value) noexcept {
		store = value;
		return *this;
	}
	value_type &operator=(inert_type const& value) noexcept {
		store = value;
		return *this;
	}
	value_type &operator=(ignore_type const& value) noexcept {
		store = value;
		return *this;
	}
	value_type &operator=(gc::hard_ptr const& value) noexcept {
		store = value;
		return *this;
	}
	value_type &operator=(value_type && other) noexcept {
		if (this != std::addressof(other)) {
			store = std::move(other.store);
		}
		return *this;
	}
	value_type &operator=(gc::hard_ptr && value) noexcept {
		store = std::forward<gc::hard_ptr>(value);
		return *this;
	}
	template<typename V>
	value_type &operator=(V *value) {
		return *this = gc::hard_ptr{v};
	}

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
class pointers_type<datatype::value_type> :
	public std::integral_constant<bool, true> {};

}

}

#endif
