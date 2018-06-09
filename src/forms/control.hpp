
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
#elif !defined(SYNAFIS_FORMS_CONTROL_HPP)
#define SYNAFIS_FORMS_CONTROL_HPP
#pragma once

/**	\file forms/control.hpp
 *	\brief Flow control special forms.
 */

namespace form {

/**	\class branch
 *	\brief The branch expression uses a test to select between two path of execution.
 *	\tparam Test The expression to evaluate to determine which branch to take.
 *	\tparam Btrue The branch followed with Test evaluates true.
 *	\tparam Bfalse The branch followed with Test evaluates false.
 */
template<typename Test, typename Btrue, typename Bfalse>
class branch {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 *	\todo If compilation becomes too much of a problem, having next only appear once would help.
	 *	\todo Such is not done for now as it would require an additional operation type.
	 */
	template<typename Next>
	using impl = typename Test::impl<op::exec_branch<BTrue::impl<Next>, BFalse::impl<Next>>>;
};

/**	\class apply
 *	\brief The apply expression calls closures and continuations.
 *	\tparam Func The type expression to evaluate to get the closure to apply.
 *	\tparam Func The type expression to evaluate to get the parameters to pass to the closure.
 *	\note When not a tail expression, saves the return location.
 */
template<typename Func, typename Params>
class apply {
private:
	/**	\typedef common
	 *	\brief The common subset of tail and body.
	 *	\tparam Next The code following this expression.
	 *	\note For tail contexts, common is the only part needed.
	 */
	template<typename Next>
	using common = typename Func::impl<concat<op::stack_push, Params::impl<Next>>>;

	/**	\typedef body
	 *	\brief The non-tail context version of the expression.
	 *	\tparam Next The code following this expression.
	 *	\details Next is made into a closure to be the return location.
	 */
	template<typename Next>
	using body = typename common<std::tuple<op::stack_push,
		op::acc_closure<call_discard<Next>>, op::exec_prep, op::exec_call>>;
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 *	\note Only op::exec_call is treated as a tail indicator.
	 */
	template<typename Next>
	using impl = typename tail_select<Next, common<op::exec_call>, body<Next>>;
};

/**	\class sequence
 *	\brief The sequence expression evaluates a series of expressions in order.
 *	\tparam Steps The expression types to expand into a sequence.
 *	\details When a tail expression, the last sub-expression in the sequence will be a tail expression.
 *	\note Sub-expression are evaluated in order during execution.
 */
template<typename... Steps>
class sequence;

/**	\class sequence<Tail>
 *	\brief The last part of a sequence.
 *	\tparam Tail The last sub-expression of a sequence.
 */
template<typename Tail>
class sequence<Tail> {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename Tail::impl<Next>;
};

/**	\class sequence<Tail>
 *	\brief The recursive part of a sequence.
 *	\tparam Head The sub-expression to expand.
 *	\tparam Tail The remaining sub-expressions to expand recursively.
 */
template<typename Head, typename... Tail>
class sequence<Head, Tail...> {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename Head::impl<sequence<Tail...>::impl<Next>>;
};

}

#endif
