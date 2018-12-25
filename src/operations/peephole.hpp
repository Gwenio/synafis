
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
#elif !defined(SYNAFIS_OPERATIONS_PEEPHOLE_HPP)
#define SYNAFIS_OPERATIONS_PEEPHOLE_HPP
#pragma once

/**	\file src/operations/peephole.hpp
 *	\brief Provides declarations needed for injecting peephole optimizations.
 */

/**	\dir peephole
 *	\brief Contains the files for peephole optimizations.
 *	\details The optimizations are split into files based on complexity in
 *	\details terms of the number of operations being replaced.
 */

/**	\page operation_peephole Operations
 *	\tableofcontents
 *	\section peephole_overview Overview
 *	\details How peephole optimizations work is when the starting types
 *	\details in a tuple for block::impl match a specialization of operation::peephole,
 *	\details a different specialization is used than normal.
 *	\details
 *	\details These peephole versions will then call operation::peep_step or
 *	\details operation::peep_finish based on whether the remaining operations
 *	\details are all consumed by the optimization.
 *	\details
 *	\details Most peephole optimizations are things that would have been done
 *	\details by hand if the code implementing Synafis had its lowest level
 *	\details be higher than the pseudo assembly provided by operations.
 *	\see config::peephole
 */

namespace operation {

using datatype::block;

using datatype::state_type;

/**	\class peephole
 *	\brief Class for determining what operation patterns to optimize.
 *	\tparam Args The remaining operations in a block.
 *	\details When the default definition is used rather than a specialization,
 *	\details then peephole optimization will not occur due to SFINAE.
 *	\details
 *	\details Specializations should inherit from peepbase.
 *	\see peepbase
 *	\see operation_peephole
 */
template<typename... Args>
class peephole {
	/**	\fn peephole()
	 *	\brief Deleted.
	 */
	peephole() = delete;

	/**	\fn ~peephole()
	 *	\brief Deleted.
	 */
	~peephole() = delete;

	/**	\typedef tail
	 *	\brief The trailing operations.
	 */
	using tail = std::tuple<Args...>;
};

/**	\class peepbase
 *	\brief Base class for peephole specializations.
 *	\tparam Head A tuple of consumed operations.
 *	\tparam Tail The trailing operation.
 *	\note The member value inherited from std::integral_constant is
 *	\note used to check if tail should be passed to block::impl.
 *	\see peephole
 */
template<typename Head, typename Tail>
class peepbase : public std::integral_constant<bool, false> {
public:
	/**	\typedef head
	 *	\brief The operations consumed by the optimization.
	 */
	using head = Head;

	/**	\typedef tail
	 *	\brief The trailing operation.
	 */
	using tail = Tail;
};

/**	\class peepbase<std::enable_if_t<(sizeof...(Tail) > 1), Head>, std::tuple<Tail...>>
 *	\brief Base class for peephole specializations.
 *	\tparam Head A tuple of consumed operations.
 *	\tparam Tail The trailing operations.
 *	\details Specialization for when there are more than one trailing operation.
 *	\see operation_peephole
 *
 */
template<typename Head, typename... Tail>
class peepbase<std::enable_if_t<(sizeof...(Tail) > 1), Head>, std::tuple<Tail...>> :
	public std::integral_constant<bool, true> {
public:
	/**	\typedef head
	 *	\brief The operations consumed by the optimization.
	 */
	using head = Head;

	/**	\typedef tail
	 *	\brief The trailing operations.
	 */
	using tail = std::tuple<Tail...>;
};

/**	\class peepbase<Head, void>
 *	\brief Base class for peephole specializations with no trailing operation(s).
 *	\tparam Head A tuple of consumed operations.
 *	\see operation_peephole
 */
template<typename Head>
class peepbase<Head, void> : public std::integral_constant<bool, false> {
public:
	/**	\typedef head
	 *	\brief The operations consumed by the optimization.
	 */
	using head = Head;
};

/**	\fn peep_step(state_type &state)
 *	\brief Function for the implementation of peephole optimizations.
 *	\tparam T A tuple of operations to optimize.
 *	\param state The state of execution.
 *	\see operation_peephole
 */
template<typename T>
void peep_step(state_type &state);

/**	\fn peep_finish(state_type &state)
 *	\brief Function for the implementation of peephole optimizations.
 *	\tparam T A tuple of operations to optimize.
 *	\param state The state of execution.
 *	\returns Returns the next block to execute.
 *	\see peephole<Head, void>
 *	\see operation_peephole
 */
template<typename T>
block peep_finish(state_type &state);

}

#endif
