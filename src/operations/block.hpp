
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
#elif !defined(SYNAFIS_OPERATIONS_BLOCK_HPP)
#define SYNAFIS_OPERATIONS_BLOCK_HPP
#pragma once

/**	\file src/operations/block.hpp
 *	\brief Expands on datatypes/block.hpp by defining and specializing some functions.
 */

namespace datatype {

template<typename T>
inline void block::step(state_type &state) {
	state = T::intrinsic(*state);
}

/**	\fn block::step<operation::env_lookup<Key>>(state_type &state)
 *	\brief Step for looking up the value bound to a key in the environment.
 *	\tparam Key A type with a static member named 'value', which will be the key to lookup.
 *	\param state The state of execution.
 *	\pre Key must be bound to a value in the environment or one of its ansestors.
 *	\details Stores the retrieved value in the accumulator.
 */
template<typename Key>
void block::step<operation::env_lookup<Key>>(state_type &state) {
	state = state[Key::value];
}

/**	\fn block::step<operation::env_assign<Keys>>(state_type &state)
 *	\brief Step for binding a value to a key in the environment.
 *	\tparam Keys A type with a static member named 'value', which will be the key(s) to bind values to.
 *	\param state The state of execution.
 *	\details Whether this mutates the environment or creates a child with the binding to replace the old
 *	\details is controlled by config::mutable_env.
 *	\details If a list is provided as a key, assignment occurs recursively.
 *	\details Pairs must be matched with pairs and the head and tail will be assigned.
 *	\details Null (called in the implementation 'empty') must match against another null.
 *	\details Symbols will be bound to the matching value in the environment.
 *	\details An ignore literal will cause the matching value to be skipped.
 *	\note The state of the accumulator after this operation should be treated as undefined.
 *	\note While making unoptimized code less efficient, it allows greater opprotunity for optimizations.
 *	\see config::mutable_env
 */
template<typename Keys>
void block::step<operation::env_assign<Keys>>(state_type &state) {
	state.bind(Keys::value, *state);
}

/**	\fn block::step<operation::env_assign<ignore>>(state_type &)
 *	\brief No binding occurs when the Keys used is ignore.
 *	\see config::mutable_env
 */
template<> inline
void block::step<operation::env_assign<ignore>>(state_type &) {}

/**	\fn block::step<operation::env_swap>(state_type &state)
 *	\brief Step for swapping the current environment with the value in the accumulator.
 *	\param state The state of execution.
 *	\pre The accumulator must contain an environment.
 */
template<>
void block::step<operation::env_swap>(state_type &state);

/**	\fn block::step<operation::env_child>(state_type &state)
 *	\brief Step for getting a child of the current environment.
 *	\param state The state of execution.
 *	\details The old environment is placed in the accumulator while the child is the new environment.
 *	\details This is because typically when a child is created we are either going to use it or
 *	\details place the old environment on the stack.
 *	\note If environments are not mutable, then a child is the same as the parent for this operation.
 */
template<>
void block::step<operation::env_child>(state_type &state);

/**	\fn block::step<operation::env_fresh>(state_type &state)
 *	\brief Step for getting a fresh environment.
 *	\param state The state of execution.
 *	\details The old environment is placed in the accumulator while the fresh one is the new environment.
 *	\details This is because typically when a fresh environment is created we are either going to use it or
 *	\details place the old environment on the stack.
 *	\note If environments are not mutable, then a child is the same as the parent for this operation.
 */
template<>
void block::step<operation::env_fresh>(state_type &state);

/**	\fn block::step<operation::stack_pop>(state_type &state)
 *	\brief Step for removing the top of the stack and placing it in the accumulator.
 *	\param state The state of execution.
 *	\note Cases where the accumulator is immediately overwritten can have the assignment optimized out.
 */
template<>
void block::step<operation::stack_pop>(state_type &state);

/**	\fn block::step<operation::stack_push>(state_type &state)
 *	\brief Step for placing a copy of the accumulator on top of the stack.
 *	\param state The state of execution.
 */
template<>
void block::step<operation::stack_push>(state_type &state);

/**	\fn block::step<operation::stack_swap>(state_type &state)
 *	\brief Step for swapping the values of the accumulator and the top of the stack.
 *	\param state The state of execution.
 */
template<>
void block::step<operation::stack_swap>(state_type &state);

/**	\fn block::step<operation::stack_cons>(state_type &state)
 *	\brief Step for making a pair with the accumulator and the top of the stack.
 *	\param state The state of execution.
 *	\details Pops the top of the stack and places it in the head of a pair and
 *	\details the accumulator as the tail.
 *	\details The created pair is placed in the accumulator.
 */
template<>
void block::step<operation::stack_cons>(state_type &state);

/**	\fn block::step<operation::stack_frame>(state_type &state)
 *	\brief Step for saving the stack state to be restored later.
 *	\param state The state of execution.
 *	\details The created 'stack frame' is stored on top of the stack after saving the state.
 *	\note This is important for implementing continuations.
 */
template<>
void block::step<operation::stack_frame>(state_type &state);

/**	\fn block::step<operation::acc_data<T>>(state_type &state)
 *	\brief Step for placing a certain value in the accumulator.
 *	\tparam T A type with a static variable named 'value'.
 *	\param state The state of execution.
 *	\details The accumulator is set to T::value.
 *	\note The value does not have to be a constant.
 *	\note For example, it could be a variable used by certain intrinsic functions.
 */
template<typename T> inline
void block::step<operation::acc_data<T>>(state_type &state) {
	state = T::value;
}

/**	\fn block::step<operation::acc_closure<T>>(state_type &state)
 *	\brief Step for creating a closure.
 *	\tparam T A type to be T in block::block<T>().
 *	\param state The state of execution.
 *	\details A closure is a pair with an environment as the head and a block as the tail.
 *	\details The environment used is the current environment, and a block is created with T.
 *	\details The resulting closure is placed in the accumulator.
 */
template<typename T>
void block::step<operation::acc_closure<T>>(state_type &state) {
	state = state_type::cons(state.current(), block<T>{});
}

template<typename T>
inline block block::finish(state_type &state) {
	return T::intrinsic(*state);
}

/**	\fn block::impl<std::tuple<Args...>>(state_type &state)
 *	\brief Injects peephole optimizations.
 *	\tparam Args Types to check for a peephole optimization.
 *	\param state The state of execution.
 *	\returns Returns the next block to execute.
 *	\details Only enabled with config::peephole is true.
 *	\details
 *	\details If peephole<Args...>::head == std::tuple<Args...> then
 *	\details operation::peep_finish<std::tuple<Args...>>(state) is called.
 *	\details Otherwise, we call operation::peep_step and the return depends
 *	\details on peephole<Args...>::value.
 *	\details
 *	\details If peephole<Args...>::value is true then block::impl is returned.
 *	\details Otherwise, block::finish is used.
 */
template<typename... Args> inline
std::enable_if_t<(config::peephole &&
	std::is_same_v<operation::peephole<Args...>::head, std::tuple<Args...>>),
	block>
block::impl<std::tuple<Args...>>(state_type &state) {
	return operation::peep_finish<std::tuple<Args...>>(state);
}

template<typename... Args> inline
std::enable_if_t<(config::peephole &&
	!std::is_same_v<operation::peephole<Args...>::head, std::tuple<Args...>> &&
	operation::peephole<Args...>::value), block>
block::impl<std::tuple<Args...>>(state_type &state) {
	operation::peep_step<typename operation::peephole<Args...>::head>(state);
	return impl<typename operation::peephole<Args...>::tail>(state);
}

template<typename... Args> inline
std::enable_if_t<(config::peephole &&
	!std::is_same_v<operation::peephole<Args...>::head, std::tuple<Args...>> &&
	!operation::peephole<Args...>::value), block>
block::impl<std::tuple<Args...>>(state_type &state) {
	step<typename operation::peephole<Args...>::head>(state);
	return finish<typename operation::peephole<Args...>::tail>(state);
}

/**	\fn block::impl<std::tuple<<Head, Tail...>>(state_type &state)
 *	\brief Specialized to omit unnecessary operations.
 *	\tparam Head The current step to execute.
 *	\tparam Tail A list of operations following the set being executed.
 *	\param state The state of execution.
 *	\returns Returns the next block to execute.
 *	\details Executes step<Head>(state) and then returns impl<Tail...>(state).
 */
template<typename Head, typename... Tail> inline
std::enable_if_t<(2 <= sizeof...(Tail) &&
	std::is_same_v<operation::peephole<Head, Tail...>::tail, std::tuple<Head, Tail...>>),
	block>
block::impl<std::tuple<Head, Tail...>>(state_type &state) {
	step<Head>(state);
	return impl<std::tuple<Tail...>>(state);
}

/**	\fn block::impl<std::tuple<Head, Tail>>(state_type &state)
 *	\brief The last pair of steps in a block.
 *	\tparam Head The current step to execute.
 *	\tparam Tail The final operation in the block.
 *	\param state The state of execution.
 *	\returns Returns the next block to execute.
 *	\details The last step is preformed with finish<Tail> rather than block::impl.
 */
template<typename Head, typename Tail> inline
std::enable_if_t<(std::is_same_v<operation::peephole<Head, Tail...>::tail,
	std::tuple<Head, Tail...>>), block>
block::impl<std::tuple<Head, Tail>>(state_type &state) {
	step<Head>(state);
	return finish<Tail>(state);
}

/**	\fn block::finish<operation::exec_branch<T, F>>(state_type &state)
 *	\brief Select from two possible blocks based on the value in the accumulator.
 *	\tparam T The first possible option.
 *	\tparam F The second possible option.
 *	\param state The state of execution.
 *	\pre The accumulator of state must contain true or false.
 *	\returns Returns block<T>{} if the accumulator contains true; otherwise, block<F>{} is returned.
 */
template<typename T, typename F>
inline block
block::finish<operation::exec_branch<T, F>>(state_type &state) {
	return state ? block<T>{} : block<F>{};
}

/**	\fn block::finish<operation::exec_call>(state_type &state)
 *	\brief Calls the closure or continuation stored on top of the stack.
 *	\param state The state of execution.
 *	\details A continuation is a list of pair with a 'stack frame' in the head
 *	\details and another such pair or and closure in the tail.
 *	\details When calling a continuation we restore the first 'stack frame' encountered
 *	\details and discard the rest. The proceed to call the trailing closure.
 *	\details For closures the environment is made current and the block is returned.
 *	\pre The top of the stack must contain a closure or a continuation.
 *	\see acc_closure<T> for the definition of a closure.
 *	\see stack_frame for information about 'stack frames'.
 *	\see config::keep_call
 */
template<>
block block::finish<operation::exec_call>(state_type &state);

/**	\fn block::impl<std::tuple<operation::exec_prep, operation::exec_call>>(state_type &state)
 *	\brief Prepares the state for applying a closure.
 *	\param state The state of execution.
 *	\details Shuffle around more values than other operations can handle.
 *	\details We move the top of the stack to the accumulator.
 *	\details The previous accumulator replaces the new stack top.
 *	\details We call the value that was originally the second value on the stack.
 *	\details As this is the only case exec_prep is used, this is not considered
 *	\details an optimization to gate with config::optimize.
 *	\details This is because having more cases increases the chance for errors.
 *	\pre The accumulator must contain a closure or a continuation.
 *	\pre The second value of the stack must contain a closure or a continuation.
 *	\note When config::keep_call == true the called item is placed on the stack.
 *	\see exec_call
 *	\see config::keep_call
 */
template<>
block block::impl<std::tuple<operation::exec_prep,
	operation::exec_call>>(state_type &state);

// Remove specializations that are an error if used.
template<typename T, typename F>
void block::step<operation::exec_branch<T, F>>(state_type &) = delete;

template<>
void block::step<operation::exec_prep>(state_type &) = delete;

template<>
void block::step<operation::exec_call>(state_type &) = delete;

template<typename Key>
block block::finish<operation::env_lookup<Key>>(state_type &) = delete;

template<typename Keys>
block block::finish<operation::env_assign<Keys>>(state_type &) = delete;

template<>
block block::finish<operation::env_swap>(state_type &) = delete;

template<>
block block::finish<operation::env_child>(state_type &) = delete;

template<>
block block::finish<operation::env_fresh>(state_type &) = delete;

template<>
block block::finish<operation::stack_pop>(state_type &) = delete;

template<>
block block::finish<operation::stack_push>(state_type &) = delete;

template<>
block block::finish<operation::stack_cons>(state_type &) = delete;

template<>
block block::finish<operation::stack_frame>(state_type &) = delete;

template<typename T>
block block::finish<operation::acc_data<T>>(state_type &) = delete;

template<typename T>
block block::finish<operation::acc_closure<T>>(state_type &) = delete;

template<>
block block::finish<operation::exec_prep>(state_type &) = delete;

}

#endif
