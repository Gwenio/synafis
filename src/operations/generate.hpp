
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

#if !defined(SYNAFIS_OPERATIONS_HPP)
#include "../operations.hpp"
#elif !defined(SYNAFIS_OPERATIONS_GENERATE_HPP)
#define SYNAFIS_OPERATIONS_GENERATE_HPP
#pragma once

/**	\file src/operation/generate.hpp
 *	\brief Defines helpers for generating tuples of operations.
 *	\note Also useful for otherwise manipulating said tuples.
 */

namespace operation {

/**	\class concat_type
 *	\brief Makes the tuple containing two types.
 *	\tparam T1 The first type to concatenate.
 *	\tparam T2 The second type to concatenate.
 */
template<typename T1, typename T2>
class concat_type {
	/**	\fn concat_type()
	 *	\brief Deleted.
	 */
	concat_type() = delete;

	/**	\fn ~concat_type()
	 *	\brief Deleted.
	 */
	~concat_type() = delete;
public:
	/**	\typedef type
	 *	\brief The produced tuple type.
	 */
	using type = typename std::tuple<T1, T2>;
};

/**	\class concat_type<T1, std::tuple<T2...>>
 *	\brief Makes the tuple of a type and the types from a tuple.
 *	\tparam T1 The type to concatenate.
 *	\tparam T2 The tuple to concatenate.
 */
template<typename T1, typename... T2>
class concat_type<T1, std::tuple<T2...>> {
	/**	\fn concat_type()
	 *	\brief Deleted.
	 */
	concat_type() = delete;

	/**	\fn ~concat_type()
	 *	\brief Deleted.
	 */
	~concat_type() = delete;
public:
	/**	\typedef type
	 *	\brief The produced tuple type.
	 */
	using type = typename std::tuple<T1, T2...>;
};

/**	\class concat_type<std::tuple<T1...>, T2>
 *	\brief Makes the tuple of a type and the types from a tuple.
 *	\tparam T1 The tuple to concatenate.
 *	\tparam T2 The type to concatenate.
 */
template<typename... T1, typename T2>
class concat_type<std::tuple<T1...>, T2> {
	/**	\fn concat_type()
	 *	\brief Deleted.
	 */
	concat_type() = delete;

	/**	\fn ~concat_type()
	 *	\brief Deleted.
	 */
	~concat_type() = delete;
public:
	/**	\typedef type
	 *	\brief The produced tuple type.
	 */
	using type = typename std::tuple<T1..., T2>;
};

/**	\class concat_type<std::tuple<T1...>, std::tuple<T2...>>
 *	\brief Makes the tuple by concatenating two other tuples' types.
 *	\tparam T1 The first tuple to concatenate.
 *	\tparam T2 The second tuple to concatenate.
 */
template<typename... T1, typename... T2>
class concat_type<std::tuple<T1...>, std::tuple<T2...>> {
	/**	\fn concat_type()
	 *	\brief Deleted.
	 */
	concat_type() = delete;

	/**	\fn ~concat_type()
	 *	\brief Deleted.
	 */
	~concat_type() = delete;
public:
	/**	\typedef type
	 *	\brief The produced tuple type.
	 */
	using type = typename std::tuple<T1..., T2...>;
};

/**	\typedef concat
 *	\brief Shorthand for typename concat_type<T1, T2>::type.
 *	\tparam T1 The first type or tuple to concatenate.
 *	\tparam T2 The second type or tuple to concatenate.
 *	\see concat_type
 */
template<typename T1, typename T2>
using concat = typename concat_type<T1, T2>::type;

/**	\typedef prefix_if
 *	\brief Conditionally concatenates T1 and T2.
 *	\tparam Cond Determines if concatenation should occur.
 *	\tparam T1 The first type or tuple to concatenate.
 *	\tparam T2 The second type or tuple to concatenate.
 *	\details Concatenates if Cond is true, otherwise the type is T2.
 *	\see concat
 */
template<bool Cond, typename T1, typename T2>
using prefix_if = typename std::conditional_t<Cond, concat<T1, T2>, T2>;

/**	\typedef append_if
 *	\brief Conditionally concatenates T1 and T2.
 *	\tparam Cond Determines if concatenation should occur.
 *	\tparam T1 The first type or tuple to concatenate.
 *	\tparam T2 The second type or tuple to concatenate.
 *	\details Concatenates if Cond is true, otherwise the type is T1.
 *	\see concat
 */
template<bool Cond, typename T1, typename T2>
using append_if = typename std::conditional_t<Cond, concat<T1, T2>, T1>;

/**	\class flatten_type
 *	\brief Flattens a tuple so the contents of contained tuples are present in the result.
 *	\tparam T The tuple type to flatten.
 */
template<typename T>
class flatten_type;

/**	\class flatten_type<std::enable_if_t<(2 <= sizeof...(Tail)), std::tuple<Head, Tail...>>>
 *	\brief Flattens a tuple so the contents of contained tuples are present in the result.
 *	\tparam Head The first element to flatten.
 *	\tparam Tail The remaining elements to flatten.
 */
template<typename Head, typename... Tail>
class flatten_type<std::enable_if_t<(2 <= sizeof...(Tail)), std::tuple<Head, Tail...>>> :
	public concat_type<Head, flatten_type<std::tuple<Tail...>>::type> {};

/**	\class flatten_type
 *	\brief Flattens two tuple elements to form a single tuple.
 *	\tparam Head The first element to flatten.
 *	\tparam Tail The remaining element to flatten.
 */
template<typename Head, typename Tail>
class flatten_type<std::tuple<Head, Tail>> : public concat_type<Head, Tail> {};

/**	\typedef flatten
 *	\brief Shorthand for typename flatten_type<std::tuple<Types...>>::type.
 *	\tparam Types The types to flatten into a single tuple.
 *	\note If there is only two types then it is equivalent to concat<Types...>.
 *	\note As such using concat directly in such cases may reduce compile time.
 *	\see flatten_type
 */
template<typename... Types>
using flatten = typename flatten_type<std::tuple<Types...>>::type;

}

#endif
