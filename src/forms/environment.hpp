
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

#ifndef SYNAFIS_FORMS_HPP
#include "../forms.hpp"
#endif

#ifndef SYNAFIS_FORMS_ENVIRONMENT_HPP
#define SYNAFIS_FORMS_ENVIRONMENT_HPP
#pragma once

/**	\file src/forms/environment.hpp
 *	\brief Environment interaction special forms.
 */

namespace form {

/**	\class lookup
 *	\brief The lookup expression fetches the value associated with a key from the current environment.
 *	\tparam K A type with a static member 'value' to be the key used in the lookup operation.
 */
template<typename K>
class lookup {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename concat<op::env_lookup<K>, Next>;
};

/**	\class assign
 *	\brief An expression template for binding value(s) to key(s) in the environment.
 *	\tparam K A type with a static member 'value' to be the key(s) used in the assign operation.
 *	\tparam V An expression template to evaluate to get the value(s) to bind.
 */
template<typename K, typename V>
class assign {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename V::impl<concat<op::env_assign<K>, Next>>;
};

/**	\class assign<std::enable_if_t<config::remove_env_noop, lit::ignore>, V>
 *	\brief Specialized to remove unneeded operations.
 *	\tparam V An expression template to evaluate to get the value(s) to bind.
 */
template<typename V>
class assign<std::enable_if_t<config::remove_env_noop, lit::ignore>, V> {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename V::impl<Next>;
};

/**	\class assign<std::enable_if_t<config::remove_env_noop, lit::ignore>, literal<V>>
 *	\brief Specialized to remove unneeded operations.
 *	\tparam V Placeholder for the type literal is specialized with.
 */
template<typename V>
class assign<std::enable_if_t<config::remove_env_noop, lit::ignore>, literal<V>> {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 *	\details Is equivalent to Next.
	 */
	template<typename Next>
	using impl = Next;
};

/**	\class assign<std::enable_if_t<config::remove_env_noop, lit::ignore>, lookup<K>>
 *	\brief Specialized to remove unneeded operations.
 *	\tparam K Placeholder for the type key for lookup.
 */
template<typename V>
class assign<std::enable_if_t<config::remove_env_noop, lit::ignore>, lookup<K>> {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 *	\details Is equivalent to Next.
	 */
	template<typename Next>
	using impl = Next;
};

/**	\class scope_common
 *	\brief A common portion of expressions that scope bindings.
 *	\tparam Op The operation type for making the new environment.
 *	\tparam K The keys to bind value(s) to in the new environment.
 *	\tparam V The expression for getting the value(s) to bind.
 *	\tparam B The expression to evaluate in the scope.
 *	\note V is evaluated in the original environment before creating the new one.
 */
template<typename Op, typename K, typename V, typename B>
class scope_common {
static_assert(std::is_same_v<op::env_child, Op> || std::is_same_v<op::env_fresh, Op>,
	"Op must be op::env_child or op::env_fresh.");
protected:
	/**	\typedef common
	 *	\brief The common subset being specialized.
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using common = typename V::impl<flatten<op::stack_push, Op,
		tail_select<Next, op::stack_pop, op::stack_swap>,
		op::env_assign<K>, B::impl<Next>>>;
};

/**	\class scope_common<std::enable_if_t<config::remove_env_noop, lit::ignore>, V, B>
 *	\brief Specialized to remove unneeded operations.
 *	\tparam Op The operation type for making the new environment.
 *	\tparam V The expression for getting the value(s) to bind.
 *	\tparam B The expression to evaluate in the scope.
 *	\note V is evaluated in the original environment before creating the new one.
 */
template<typename Op, typename V, typename B>
class scope_common<std::enable_if_t<config::remove_env_noop, lit::ignore>, V, B> {
static_assert(std::is_same_v<op::env_child, Op> || std::is_same_v<op::env_fresh, Op>,
	"Op must be op::env_child or op::env_fresh.");
protected:
	/**	\typedef common
	 *	\brief The common subset being specialized.
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using common = typename V::impl<concat<Op,
		prefix_if<std::is_same_v<op::exec_call, Next>,
			op::stack_push, B::impl<Next>>>;
};

/**	\class scope_common<std::enable_if_t<config::remove_env_noop, lit::ignore>, literal<V>, B>
 *	\brief Specialized to remove unneeded operations.
 *	\tparam Op The operation type for making the new environment.
 *	\tparam V Placeholder for the type literal is specialized with.
 *	\tparam B The expression to evaluate in the scope.
 */
template<typename Op, typename V, typename B>
class scope_common<std::enable_if_t<config::remove_env_noop, lit::ignore>, literal<V>, B> {
static_assert(std::is_same_v<op::env_child, Op> || std::is_same_v<op::env_fresh, Op>,
	"Op must be op::env_child or op::env_fresh.");
protected:
	/**	\typedef common
	 *	\brief The common subset being specialized.
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using common = typename concat<Op,
		prefix_if<std::is_same_v<op::exec_call, Next>,
			op::stack_push, B::impl<Next>>;
};

/**	\class scope_common<std::enable_if_t<config::remove_env_noop, lit::ignore>, lookup<K>, B>
 *	\brief Specialized to remove unneeded operations.
 *	\tparam Op The operation type for making the new environment.
 *	\tparam K Placeholder for the type key for lookup.
 *	\tparam B The expression to evaluate in the scope.
 */
template<typename Op, typename K, typename B>
class scope_common<std::enable_if_t<config::remove_env_noop, lit::ignore>, lookup<K>, B> {
static_assert(std::is_same_v<op::env_child, Op> || std::is_same_v<op::env_fresh, Op>,
	"Op must be op::env_child or op::env_fresh.");
protected:
	/**	\typedef common
	 *	\brief The common subset being specialized.
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using common = typename concat<Op,
		prefix_if<std::is_same_v<op::exec_call, Next>,
			op::stack_push, B::impl<Next>>;
};

/**	\class child
 *	\brief The expression for evaluating a sub-expression in a child environment.
 *	\tparam K The keys to bind values to in the new environment.
 *	\tparam V The expression for getting the values to bind.
 *	\tparam B The expression to evaluate in the scope.
 *	\see scope_common
 */
template<typename K, typename V, typename B>
class child : public scope_common<op::env_child, K, V, B> {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename tail_select<Next, common<Next>,
		common<concat<restore_env, Next>>>;
};

/**	\class fresh
 *	\brief The expression for evaluating a sub-expression in a fresh environment.
 *	\tparam K The keys to bind values to in the new environment.
 *	\tparam V The expression for getting the values to bind.
 *	\tparam B The expression to evaluate in the scope.
 *	\see scope_common
 */
template<typename K, typename V, typename B>
class fresh: public scope_common<op::env_fresh, K, V, B> {
public:
	/**	\typedef impl
	 *	\brief The type to pass to block::block<T>().
	 *	\tparam Next The code following this expression.
	 */
	template<typename Next>
	using impl = typename tail_select<Next, common<Next>,
		common<concat<restore_env, Next>>>;
};

}

#endif
