
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

#ifndef SYNAFIS_OPERATIONS_HPP
#include "../operations.hpp"
#endif

#ifndef SYNAFIS_OPERATIONS_PEEPHOLE_HPP
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

/**	\page operation_peephole Peephole
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

	/**
	 *	\brief The trailing operations.
	 */
	typedef std::tuple<Args...> tail;
};

/**	\class peepbase
 *	\brief Base class for peephole specializations.
 *	\tparam Head A tuple of consumed operations.
 *	\tparam Tail The trailing operation.
 *	\tparam Enable Used to enable specializations with std::enable_if_t.
 *	\note The member value inherited from std::integral_constant is
 *	\note used to check if tail should be passed to block::impl.
 *	\see peephole
 */
template<typename Head, typename Tail, typename Enable = void>
class peepbase : public std::integral_constant<bool, false> {
public:
	/**
	 *	\brief The operations consumed by the optimization.
	 */
	typedef Head head;

	/**
	 *	\brief The trailing operation.
	 */
	typedef Tail tail;
};

/**
 *	\brief Base class for peephole specializations.
 *	\tparam Head A tuple of consumed operations.
 *	\tparam Tail The trailing operations.
 *	\details Specialization for when there are more than one trailing operation.
 *	\see peepbase\<Head, Tail, Enable\>
 *
 */
template<typename Head, typename... Tail>
class peepbase<Head, std::tuple<Tail...>, std::enable_if_t<(sizeof...(Tail) > 1)>> :
	public std::integral_constant<bool, true> {
public:
	/**
	 *	\brief The operations consumed by the optimization.
	 */
	typedef Head head;

	/**
	 *	\brief The trailing operations.
	 */
	typedef std::tuple<Tail...> tail;
};

/**
 *	\brief Base class for peephole specializations with no trailing operation(s).
 *	\tparam Head A tuple of consumed operations.
 *	\see peepbase\<Head, Tail, Enable\>
 */
template<typename Head>
class peepbase<Head, void, void> : public std::integral_constant<bool, false> {
public:
	/**
	 *	\brief The operations consumed by the optimization.
	 */
	typedef Head head;
};

/**	\fn peep_step(state_type &state)
 *	\brief Function for the implementation of peephole optimizations.
 *	\tparam T A tuple of operations to optimize.
 *	\tparam Enable Used to enable specializations with std::enable_if_t.
 *	\param state The state of execution.
 *	\see operation_peephole
 */
template<typename T, typename Enable = std::enable_if_t<config::peephole>>
void peep_step(state_type &state);

/**	\fn peep_finish(state_type &state)
 *	\brief Function for the implementation of peephole optimizations.
 *	\tparam T A tuple of operations to optimize.
 *	\tparam Enable Used to enable specializations with std::enable_if_t.
 *	\param state The state of execution.
 *	\returns Returns the next block to execute.
 *	\see peepbase\<Head\>
 *	\see operation_peephole
 */
template<typename T, typename Enable = std::enable_if_t<config::peephole>>
block peep_finish(state_type &state);

/**	\typedef enable_peep_finish
 *	\brief Condition for enabling peep_finish() to optimize a series of operations.
 *	\tparam Args The series of operations to check for.
 *	\see peep_finish(state_type &state)
 */
template<typename... Args>
using enable_peep_finish = typename std::enable_if_t<config::peephole &&
	std::is_same_v<operation::peephole<Args...>::head, std::tuple<Args...>>>;

/**	\typedef enable_peep_step
 *	\brief Condition for enabling peep_step() to optimize a series of operations.
 *	\tparam Args The series of operations to check for.
 *	\see peep_step(state_type &state)
 */
template<typename... Args>
using enable_peep_step = typename std::enable_if_t<config::peephole &&
	!std::is_same_v<operation::peephole<Args...>::head, std::tuple<Args...>> &&
	operation::peephole<Args...>::value>;

/**	\var no_peep
 *	\brief Condition for disabling peep_step() and peep_finish() to optimize a series of operations.
 *	\tparam Args The series of operations to check for.
 *	\see peep_step(state_type &state)
 *	\see peep_finish(state_type &state)
 */
template<typename... Args>
static inline constexpr bool no_peep{!config::peephole ||
	!(std::is_same_v<operation::peephole<Args...>::head, std::tuple<Args...>> ||
	operation::peephole<Args...>::value)};

}

#endif
