
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
#elif !defined(SYNAFIS_FORMS_CALLABLE_HPP)
#define SYNAFIS_FORMS_CALLABLE_HPP
#pragma once

/**	\file src/forms/callable.hpp
 *	\brief Special forms for creating closures and continuations.
 */

namespace form {

/**	\class closure_outer
 *	\brief Defines the code for creating a closure.
 *	\tparam R Specialize based on the type used for the key value of the recursive binding.
 *	\note R should always be lit::ignore if config::keep_call == true.
 */
template<typename R>
class closure_outer {
static_assert(!config::keep_call || std::is_same_v<lit::ignore , R>,
	"When calls are kept, the template parameter R must be lit::ignore.");
protected:
	/**	\typedef outer
	 *	\brief Specifies the operations to create the closure.
	 *	\tparam B The body of closure.
	 *	\tparam Next The code to execute after creating the closure.
	 */
	template<typename B, typename Next>
	using outer = typename flatten<op::env_child,
		tail_select<Next,
			std::tuple<op::acc_closure<B>, op::stack_push, op::env_assign<R>>,
			std::tuple<op::stack_push, op::acc_closure<B>, op::stack_push,
				op::env_assign<R>, op::stack_pop, op::stack_swap, op::env_swap>>,
		op::stack_pop, Next>;
};

/**	\class closure_outer<lit::ignore>
 *	\brief Optimized specialization of closure_outer<R>.
 *	\note This specialization should always be used if config::keep_call == true.
 */
template<>
class closure_outer<lit::ignore> {
protected:
	/**	\typedef outer
	 *	\brief Specifies the operations to create the closure.
	 *	\tparam B The body of closure.
	 *	\tparam Next The code to execute after creating the closure.
	 */
	template<typename B, typename Next>
	using outer = typename concat<op::acc_closure<B>, Next>;
};

/**	\class closure_inner
 *	\brief Defines the code for the body of closures.
 *	\tparam P The type for the key(s) to bind parameters to.
 */
template<typename P>
class closure_inner {
protected:
	/**	\typedef inner
	 *	\brief The type for the operations for the closure body.
	 *	\tparam R Specialize based on the type used for the key value of the recursive binding.
	 *	\tparam B The expression type for the body of the closure.
	 */
	template<typename R, typename B>
	using inner = typename flatten<child_env,
		prefix_if<config::keep_call,
			std::conditional_t<std::is_same_v<R, lit::ignore>,
				stack_discard,
				std::tuple<op::stack_swap, op::env_assign<R>, op::stack_pop>>,
			concat<op::env_assign<P>, B::impl<op::exec_call>>>;
};

/**	\class closure_inner<lit::ignore>
 *	\brief Optimized specialization of closure_inner<P>.
 */
template<>
class closure_inner<lit::ignore> {
protected:
	/**	\typedef inner
	 *	\brief The type for the operations for the closure body.
	 *	\tparam R Specialize based on the type used for the key value of the recursive binding.
	 *	\tparam B The expression type for the body of the closure.
	 */
	template<typename R, typename B>
	using inner = typename flatten<op::env_child,
		prefix_if<config::keep_call,
			std::conditional_t<std::is_same_v<R, lit::ignore>,
				stack_discard,
				std::tuple<op::stack_pop, op::env_assign<R>>>,
			B::impl<op::exec_call>>;
};

/**	\class closure
 *	\brief The closure expression creates a new closure.
 *	\tparam R Specialize based on the type used for the key value of the recursive binding.
 *	\tparam P The type for the key(s) to bind parameters to.
 *	\tparam B The expression type for the body of the closure.
 */
template<typename R, typename P, typename B>
class closure : public
	closure_outer<std::conditional_t<config::keep_call, lit::ignore, R>>,
	closure_inner<P> {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename outer<inner<R, B>, Next>;
};

/**	\class letcc
 *	\brief The letcc expression creates a continuation and defines it for the enclosed scope.
 *	\tparam K The key to bind the continuation to.
 *	\tparam B The sub-expression to evaluate in a child environment where the continuation is bound.
 */
template<typename K, typename B>
class letcc {
static_assert(!std::is_same_v<K, lit::ignore>,
	"It is incorrect to bind a continuation to ignore.");
static_assert(!std::is_same_v<K, lit::empty>,
	"It is incorrect to bind a continuation to empty.");
private:
	/**	\typedef common
	 *	\brief The common subset of tail and body.
	 *	\details Enter a child environment if needed and then assign the continuation to K.
	 *	\note When environments are not mutable, we may remove child_env.
	 */
	using common = typename concat<
		std::conditional_t<(config::remove_env_noop && !config::mutable_env)
			std::tuple<op::stack_push, op::assign<K>>,
			flatten<op::stack_push, child_env, op::assign<K>>>,
		B::impl<op::exec_call>>;

	/**	\typedef tail
	 *	\brief The tail context version of the expression.
	 *	\details In tail context, create the frame with the return location.
	 *	\note When calls are kept, leave the return location on top of the stack.
	 */
	using tail = typename flatten<std::conditional_t<config::keep_call, stack_top, op::stack_pop>,
		op::stack_frame, op::stack_cons, common>;

	/**	\typedef body
	 *	\brief The non-tail context version of the expression.
	 *	\details Create the frame with a new closure.
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using body = typename flatten<op::stack_frame, op::acc_closure<Next>, op::stack_cons, common>;
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename tail_select<Next, tail, body<Next>>;
};

}

#endif
