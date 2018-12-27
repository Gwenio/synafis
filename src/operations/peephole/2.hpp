
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

#ifndef SYNAFIS_OPERATIONS_PEEPHOLE_HPP
#include "../peephole.hpp"
#endif

#ifndef SYNAFIS_OPERATIONS_PEEPHOLE_2_HPP
#define SYNAFIS_OPERATIONS_PEEPHOLE_2_HPP
#pragma once

/**	\file src/operations/peephole/2.hpp
 *	\brief Defines optimizations of various patterns of two operations.
 *	\note These specializations should only be present when config::peephole == true.
 *	\todo Add more optimization cases. The current set is more an example than a meaningful set.
 *	\see config::peephole
 */

namespace operation {

/**
 *	\brief Specialization.
 *	\tparam Tail The trailing steps.
 *	\see peephole
 *	\see peep_step\<std::tuple\<stack_swap, stack_pop\>\>(state_type &state)
 */
template<typename... Tail>
class peephole<stack_swap, stack_pop, Tail...> :
	public peepbase<std::tuple<stack_swap, stack_pop>, std::tuple<Tail...>> {};

/**
 *	\brief Optimizes to remove the top of the stack without other state changes.
 *	\param state The state of execution.
 *	\see config::peephole
 */
extern template void
peep_step<std::tuple<stack_swap, stack_pop>>(state_type &state);

/**
 *	\brief Specialization.
 *	\tparam Tail The trailing steps.
 *	\see peephole
 *	\see peep_step\<std::tuple\<stack_pop, stack_push\>\>(state_type &state)
 */
template<typename... Tail>
class peephole<stack_pop, stack_push, Tail...> :
	public peepbase<std::tuple<stack_pop, stack_push>, std::tuple<Tail...>> {};

/**
 *	\brief Optimizes to set the accumulator to equal the top of the stack.
 *	\param state The state of execution.
 *	\see config::peephole
 */
extern template void
peep_step<std::tuple<stack_pop, stack_push>>(state_type &state);

}

#endif
