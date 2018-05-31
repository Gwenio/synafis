
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

#if !defined(SYNAFIS_DATATYPES_HPP)
#include "../datatypes.hpp"
#elif !defined(SYNAFIS_DATATYPES_STATE_TYPE_HPP)
#define SYNAFIS_DATATYPES_TYPE_HPP
#pragma once

/**	\file datatypes/state_type.hpp
 *	\brief Defines a type to manage execution state.
 */

namespace datatype {

/**	\class state_type
 *	\brief A type for managing execution state.
 *	\details Rather than directly implement the basic Synafis operations, the interface for
 *	\details state_type exposes tools for implementing them so that further specializations
 *	\details of block::impl can be created that optimize special cases.
 *	\todo Document once the interface is more stable.
 */
class state_type {
	friend unit_test::tester<state_type>;
private:
	/**	\var accumulator
	 *	\brief Store the accumulated result of operations so far.
	 *	\details Used for passing parameters and return values.
	 */
	value_type accumulator;

	/**	\var environment
	 *	\brief The current environment.
	 */
	environment_type *environment;

	std::tuple<bool, block> call(value_type value);
public:
	/**	\fn state_type()
	 *	\brief Registers this as a root object with the garbage collector.
	 */
	state_type();

	/**	\fn ~state_type()
	 *	\brief Unregisters this as a root object with the garbage collector.
	 */
	~state_type();

	/**	\fn operator bool() const
	 *	\brief Implicit convertion to a boolean value.
	 */
	operator bool() const {
		return static_cast<bool>(accumulator);
	}

	/**	\fn operator*() const
	 *	\brief Accesses the accumulator.
	 */
	value_type operator*() const {
		return accumulator;
	}

	template<typename V>
	state_type &operator=(V const& value) {
		accumulator = value;
		return *this;
	}

	template<typename V>
	state_type &operator=(V && value) {
		accumulator = std::forward<V>(value);
		return *this;
	}

	/**	\fn operator()()
	 *	\brief Call the value on the top of the stack.
	 *	\returns Returns the next block to execute.
	 *	\details Sets the environment and the stack frame as needed.
	 *	\details Removes the top of the stack if !config::keep_call and we did not restore a frame.
	 *	\see config::keep_call
	 */
	block operator()();

	/**	\fn operator()(value_type value)
	 *	\brief Call the passed value.
	 *	\param value The value to call.
	 *	\returns Returns the next block to execute.
	 *	\details Sets the environment and the stack frame as needed.
	 *	\details Places value on top of the stack if config::keep_call and we did not restore a frame.
	 *	\see config::keep_call
	 */
	block operator()(value_type value);

	template<typename K>
	value_type operator[](K const& key) const {
		return environment[key];
	}

	template<typename K>
	void bind(K const& keys, value_type const& value) {
		environment.bind(keys, value);
	}

	/**	\fn env(void *e)
	 *	\brief Sets the environment to use.
	 */
	void env(environment_type e);

	/**	\fn current() const
	 *	\brief Accesses the current environment.
	 */
	environment_type current() const;

	/**	\fn child() const
	 *	\brief Provides a child of the current environment.
	 */
	environment_type child() const;

	/**	\fn fresh()
	 *	\brief Creates a blank environment.
	 */
	static environment_type fresh();

	/**	\fn top() const
	 *	\brief Accesses the value on top of the stack.
	 *	\returns Returns the value on top of the stack.
	 */
	value_type top() const;

	/**	\fn void pop()
	 *	\brief Removes the value at the top of the stack.
	 */
	void pop();

	/**	\fn push(value_type const& value)
	 *	\brief Places a value on top of the stack.
	 *	\param value The value to place on top of the stack.
	 */
	void push(value_type const& value);

	/**	\fn frame() const
	 *	\brief Saves the current stack to be restored later.
	 */
	value_type frame() const;

	static value_type cons(value_type const& v1, value_type const& v2);

	/**	\fn traverse(void *arg, gc::enumerate_cb cb) const noexcept
	 *	\brief Traverses pointers for the garbage collector.
	 *	\param arg Optional parameter to pass to cb.
	 *	\param cb The callback to enumerate pointers.
	 */
	void traverse(void *arg, gc::enumerate_cb cb) const noexcept;

	/**	\fn remap(void *arg, gc::remap_cb cb) noexcept
	 *	\brief Remaps pointers for the garbage collector.
	 *	\param arg Optional parameter to pass to cb.
	 *	\param cb The callback to remap pointers.
	 */
	void remap(void *arg, gc::remap_cb cb) noexcept;
};

}

namespace gc {

namespace traits {

template<>
class pointers_type<datatype::state_type> :
	public std::integral_constant<bool, true> {};

}

}

#endif
