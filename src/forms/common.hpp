
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

#if !defined(SYNAFIS_FORMS_HPP)
#include "../forms.hpp"
#elif !defined(SYNAFIS_FORMS_COMMON_HPP)
#define SYNAFIS_FORMS_COMMON_HPP
#pragma once

/**	\file forms/common.hpp
 *	\brief Helpers for writing special forms.
 *	\details Template heavy code benefits from minimizing variation.
 *	\details This header standardizes the sequence of operations for frequent
 *	\details use cases via typedefs.
 *	\note Imports the helpers from "operations/generate.hpp" to the forms namespace.
 */

namespace form {

/**	\namespace op
 *	\brief Shorthand for operation for the operation types.
 */
namespace op {

using operation::env_lookup;
using operation::env_assign;
using operation::env_swap;
using operation::env_child;
using operation::env_fresh;
using operation::stack_pop;
using operation::stack_push;
using operation::stack_swap;
using operation::stack_cons;
using operation::stack_frame;
using operation::acc_data;
using operation::acc_closure;
using operation::exec_branch;
using operation::exec_prep;
using operation::exec_call;

}

using operation::concat;
using operation::flatten;
using operation::append_if;
using operation::prefix_if;

/**	\class is_tail
 *	\brief Template for determining if the tail of a block.
 *	\tparam T The type to check for indicating a tail context.
 *	\details For types indicating a tail context, specialize inheriting from std::true_type.
 *	\note It is expected that other than exec_call, the other tail indicators will be intrinsics.
 */
template<typename T>
class is_tail : public std::false_type {};

/**	\class is_tail<op::exec_call>
 *	\brief exec_call is the standard tail indicator.
 */
template<>
class is_tail<op::exec_call> : public std::true_type {};

/**	\typedef tail_select
 *	\brief Simplifies selecting whether to use the tail context version of a form or not.
 *	\tparam Next The following code.
 *	\tparam Tail The tail context implementation.
 *	\tparam Body The non-tail context implementation.
 */
template<typename Next, typename Tail, typename Body>
using tail_select = typename std::conditional_t<is_tail<Next>::value, Tail, Body>;

/**	\typedef stack_discard
 *	\brief Discard the top value of the stack without changing other aspects of state.
 */
using stack_discard = std::tuple<op::stack_swap, op::stack_pop>;

/**	\typedef call_discard
 *	\brief Prefix Next with stack_discard if calls are kept.
 *	\tparam Next The operation(s) to prefix.
 *	\see config::keep_call
 */
template<typename Next>
using call_discard = typename prefix_if<config::keep_call, stack_discard, Next>;

/**	\typedef stack_top
 *	\brief Place the top of the stack in the accumulator without removing it.
 */
using stack_top = std::tuple<op::stack_pop, op::stack_push>;

/**	\typedef restore_env
 *	\brief Switch to an environment previously saved on the stack while preserving the accumulator.
 */
using restore_env = std::tuple<op::stack_swap, op::env_swap, op::stack_pop>;

/**	\typedef child_env
 *	\brief Change to a child of the current environment while preserving the accumulator.
 *	\note This sequence of operations has no affect on execution state if environments are immutable.
 */
using child_env = std::tuple<op::stack_push, op::env_child, op::stack_pop>;

}

#endif
