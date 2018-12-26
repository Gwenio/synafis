
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

#ifndef SYNAFIS_DATATYPES_HPP
#include "../datatypes.hpp"
#endif

#ifndef SYNAFIS_DATATYPES_LITERALS_HPP
#define SYNAFIS_DATATYPES_LITERALS_HPP
#pragma once

/**	\file src/datatypes/literals.hpp
 *	\brief Defines the types for literals.
 */

namespace datatype {

/**	\class empty_type
 *	\brief The literal representing an empty_type list or signaling the end of one.
 */
class empty_type {
public:
	/**	\fn empty_type() noexcept
	 *	\brief Default.
	 */
	constexpr empty_type() noexcept = default;
	/**	\fn empty_type(empty_type const&) noexcept
	 *	\brief Default.
	 */
	constexpr empty_type(empty_type const&) noexcept = default;
	/**	\fn empty_type(empty_type &&) noexcept
	 *	\brief Default.
	 */
	constexpr empty_type(empty_type &&) noexcept = default;
	/**	\fn ~empty_type() noexcept
	 *	\brief Default.
	 */
	~empty_type() noexcept = default;
	/**	\fn operator=(empty_type const&) noexcept
	 *	\brief No change actually occurs.
	 *	\returns *this
	 */
	constexpr empty_type &operator=(empty_type const&) noexcept {
		return *this;
	}
	/**	\fn operator=(empty_type &&) noexcept
	 *	\brief No change actually occurs.
	 *	\returns *this
	 */
	constexpr empty_type &operator=(empty_type &&) noexcept {
		return *this;
	}
	/**	\fn operator==(empty_type const&) const noexcept
	 *	\brief Always true.
	 *	\returns true
	 */
	constexpr bool operator==(empty_type const&) const noexcept {
		return true;
	}
	/**	\fn operator!=(empty_type const&) const noexcept
	 *	\brief Always false.
	 *	\returns false
	 */
	constexpr bool operator!=(empty_type const&) const noexcept {
		return false;
	}
};

/**	\typedef empty
 *	\brief Type to use as a template parameter to operations.
 */
typedef std::integral_constant<empty_type, empty_type{}> empty;

/**	\class inert_type
 *	\brief The literal for 'no value'. Equivalent to 'void' in C.
 *	\details Mainly exists to be returned by closures as a return value with no meaning implied.
 */
class inert_type {
public:
	/**	\fn inert_type() noexcept
	 *	\brief Default.
	 */
	constexpr inert_type() noexcept = default;
	/**	\fn inert_type(inert_type const&) noexcept
	 *	\brief Default.
	 */
	constexpr inert_type(inert_type const&) noexcept = default;
	/**	\fn inert_type(inert_type &&) noexcept
	 *	\brief Default.
	 */
	constexpr inert_type(inert_type &&) noexcept = default;
	/**	\fn ~inert_type() noexcept
	 *	\brief Default.
	 */
	~inert_type() noexcept = default;
	/**	\fn operator=(inert_type const&) noexcept
	 *	\brief No change actually occurs.
	 *	\returns *this
	 */
	constexpr inert_type &operator=(inert_type const&) noexcept {
		return *this;
	}
	/**	\fn operator=(inert_type &&) noexcept
	 *	\brief No change actually occurs.
	 *	\returns *this
	 */
	constexpr inert_type &operator=(inert_type &&) noexcept {
		return *this;
	}
	/**	\fn operator==(inert_type const&) const noexcept
	 *	\brief Always true.
	 *	\returns true
	 */
	constexpr bool operator==(inert_type const&) const noexcept {
		return true;
	}
	/**	\fn operator!=(inert_type const&) const noexcept
	 *	\brief Always false.
	 *	\returns false
	 */
	constexpr bool operator!=(inert_type const&) const noexcept {
		return false;
	}
};

/**	\typedef inert
 *	\brief Type to use as a template parameter to operations.
 */
typedef std::integral_constant<inert_type, inert_type{}> inert;

/**	\class ignore_type
 *	\brief The literal for ignoring values when matching one list against another.
 *	\details When binding value(s) to key(s) in an environment, ignore_type as a key
 *	\details causes the paired value to be skipped.
 */
class ignore_type {
public:
	/**	\fn ignore_type() noexcept
	 *	\brief Default.
	 */
	constexpr ignore_type() noexcept = default;
	/**	\fn ignore_type(ignore_type const&) noexcept
	 *	\brief Default.
	 */
	constexpr ignore_type(ignore_type const&) noexcept = default;
	/**	\fn ignore_type(ignore_type &&) noexcept
	 *	\brief Default.
	 */
	constexpr ignore_type(ignore_type &&) noexcept = default;
	/**	\fn ~ignore_type() noexcept
	 *	\brief Default.
	 */
	~ignore_type() noexcept = default;
	/**	\fn operator=(ignore_type const&) noexcept
	 *	\brief No change actually occurs.
	 *	\returns *this
	 */
	constexpr ignore_type &operator=(ignore_type const&) noexcept {
		return *this;
	}
	/**	\fn operator=(ignore_type &&) noexcept
	 *	\brief No change actually occurs.
	 *	\returns *this
	 */
	constexpr ignore_type &operator=(ignore_type &&) noexcept {
		return *this;
	}
	/**	\fn operator==(ignore_type const&) const noexcept
	 *	\brief Always true.
	 *	\returns true
	 */
	constexpr bool operator==(ignore_type const&) const noexcept {
		return true;
	}
	/**	\fn operator!=(ignore_type const&) const noexcept
	 *	\brief Always false.
	 *	\returns false
	 */
	constexpr bool operator!=(ignore_type const&) const noexcept {
		return false;
	}
};

/**	\typedef ignore
 *	\brief Type to use as a template parameter to operations.
 */
typedef std::integral_constant<ignore_type, ignore_type{}> ignore;

/**	\typedef true_
 *	\brief Type to use as a template parameter to operations.
 */
typedef std::integral_constant<bool, true> true_;

/**	\typedef false_
 *	\brief Type to use as a template parameter to operations.
 */
typedef std::integral_constant<bool, false> false_;

}

#endif
