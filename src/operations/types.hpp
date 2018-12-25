
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
#elif !defined(SYNAFIS_OPERATIONS_TYPES_HPP)
#define SYNAFIS_OPERATIONS_TYPES_HPP
#pragma once

/**	\file src/operations/types.hpp
 *	\brief Defines the operation types.
 *	\details An operation type is a type used to specialize block::step or block::finish.
 *	\note These types are only declared and not defined.
 *	\note This is because they are only needed as template parameters, they are never constructed.
 */

namespace operation {

/**	\class env_lookup
 *	\brief Operation type for looking up the value bound to a key.
 *	\tparam Key A type with a static member named 'value', which will be the key to lookup.
 */
template<typename Key>
class env_lookup;

/**	\class env_assign
 *	\brief Operation type for binding values to keys in the current environment.
 *	\tparam Keys A type with a static member named 'value', which will be the key(s) to bind values to.
 */
template<typename Keys>
class env_assign;

/**	\class env_swap
 *	\brief Operation type for swapping the current environment and the accumulator.
 */
class env_swap;

/**	\class env_child
 *	\brief Operation type for making a child of the current environment be current.
 */
class env_child;

/**	\class env_fresh
 *	\brief Operation type for making a blank environment be current.
 */
class env_fresh;

/**	\class stack_pop
 *	\brief Operation type for moving the top of the stack to the accumulator.
 */
class stack_pop;

/**	\class stack_push
 *	\brief Operation type for placing a value on top of the stack.
 */
class stack_push;

/**	\class stack_swap
 *	\brief Operation type for swapping the accumulator and the top of the stack.
 */
class stack_swap;

/**	\class stack_cons
 *	\brief Operation type for making a pair.
 */
class stack_cons;

/**	\class stack_frame
 *	\brief Operation type for saving the stack state to be restored later.
 */
class stack_frame;

/**	\class acc_data
 *	\brief Operation type setting the accumulator.
 *	\tparam T The accumulator is set to T::value.
 */
template<typename T>
class acc_data;

/**	\class acc_closure
 *	\brief Operation type for making closures.
 *	\tparam T The type to specialize the block constructor with.
 */
template<typename T>
class acc_closure;

/**	\class exec_branch
 *	\brief Operation type for selecting between two following blocks.
 *	\tparam T The first possible option.
 *	\tparam F The second possible option.
 */
template<typename T, typename F>
class exec_branch;

/**	\class exec_prep
 *	\brief Operation type for preparing for applying a closure.
 *	\note This operation is currently only expected immediately preceding exec_call.
 */
class exec_prep;

/**	\class exec_call
 *	\brief Operation type for calling closures and continuations.
 */
class exec_call;

}

#endif
