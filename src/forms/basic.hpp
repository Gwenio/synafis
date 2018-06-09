
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
#elif !defined(SYNAFIS_FORMS_BASIC_HPP)
#define SYNAFIS_FORMS_BASIC_HPP
#pragma once

/**	\file forms/basic.hpp
 *	\brief Some simple special forms.
 */

namespace form {

/**	\class literal
 *	\brief The literal expression places its value in the accumulator.
 *	\tparam V The type to use as a template parameter for op::acc_data.
 */
template<typename V>
class literal {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename concat<op::acc_data<V>, Next>;
};

/**	\class invoke
 *	\brief The invoke expression calls intrinsic functions.
 *	\tparam I Type for an intrinsic function.
 *	\tparam Params The expression for the parameters to pass to the intrinsic.
 */
template<typename I, typename Params>
class invoke {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 *	\details If Next is void, then I is the last operation in the block.
	 */
	template<typename Next>
	using impl = typename Params::impl<
		std::conditional_t<(std::is_same_v<Next, void>),
		I, concat<I, Next>>>;
};

/**	\class cons
 *	\brief The cons expression makes a pair from the results of two sub-expressions.
 *	\tparam Head The first sub-expression, the value is placed in the head of the pair.
 *	\tparam Tail The second sub-expression, the value is placed in the tail of the pair.
 *	\note Head is evaluated before Tail during execution.
 */
template<typename Head, typename Tail>
class cons {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename Head::impl<concat<op::stack_push,
		Tail::impl<concat<op::stack_cons, Next>>>>;
};

}

#endif
