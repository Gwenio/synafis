
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

#ifndef SYNAFIS_DATATYPES_HPP
#include "../datatypes.hpp"
#endif

#ifndef SYNAFIS_DATATYPES_BLOCK_HPP
#define SYNAFIS_DATATYPES_BLOCK_HPP
#pragma once

/**	\file src/datatypes/block.hpp
 *	\brief Defines a type to represent a block of meta code.
 */

namespace datatype {

class state_type;

/**	\class block
 *	\brief Represents a block of meta-assembled code.
 */
class block {
	//!	\cond friends
	// for so internals can be accessed for testing
	friend unit_test::tester<block>;
	//!	\endcond
private:
	/**	\var ptr
	 *	\brief A pointer to the block's code.
	 */
	block (*ptr)(state_type &);

	/**	\fn impl(state_type &state)
	 *	\brief The implementation for a block's code.
	 *	\tparam T A tuple of types controlling operations preformed on state.
	 *	\tparam Enable Used to enable specializations with std::enable_if_t.
	 *	\param state The state of execution.
	 *	\returns Returns the next block to execute.
	 *	\note Specialize the template to create optimizations for code patterns.
	 *	\warning All specializations need to have been declared before use.
	 *	\see operations/block.hpp for the definition.
	 *	\see config::optimize
	 */
	template<typename T, typename Enable = void>
	static block impl(state_type &state);

	/**	\fn step(state_type &state)
	 *	\brief The implementation for intermediate steps, defaulting to treating as an intrinsic.
	 *	\tparam T The type representing the operation to preform on state.
	 *	\param state The state of execution.
	 *	\details An intrinsic is expected to have a static member function called
	 *	\details 'intrinsic' that accepts a value_typ as a parameter.
	 *	\details
	 *	\details The return value will be stored in the accumulator of state.
	 *	\details
	 *	\details The passed parameter will be the previous value of the accumulator.
	 *	\note Types that should only be at the end of a tuple passed to impl should
	 *	\note have their specialization of this function deleted to avoid accidents.
	 *	\see operations/block.hpp for the definition.
	 */
	template<typename T>
	static void step(state_type &state);

	/**	\fn finish(state_type &state)
	 *	\brief The implementation for the final step in a block, defaulting to treating as an intrinsic.
	 *	\tparam T The type representing the operation to preform on state.
	 *	\param state The state of execution.
	 *	\returns Returns the next block to execute.
	 *	\returns For intrinsics, this will be the return value of the intrinsic.
	 *	\details An intrinsic is expected to have a static member function called
	 *	\details 'intrinsic' that accepts a value_typ as a parameter.
	 *	\details
	 *	\details The passed parameter will be the previous value of the accumulator.
	 *	\note Types that should only be at the end of a tuple passed to impl should
	 *	\note have their specialization of this function deleted to avoid accidents.
	 *	\see operations/block.hpp for the definition.
	 */
	template<typename T>
	static block finish(state_type &state);
public:
	/**	\fn block() noexcept
	 *	\brief Member ptr is set to &block::impl\<T\>.
	 *	\tparam T The type for block::impl\<T\>.
	 */
	template<typename T>
	constexpr block() noexcept : ptr(&block::impl<T>) {}

	/**	\fn block(block const& other) noexcept
	 *	\brief Default.
	 *	\param other The block to copy.
	 */
	constexpr block(block const& other) noexcept = default;

	/**	\fn block(block && other) noexcept
	 *	\brief Default.
	 *	\param other The block to move to the new one.
	 */
	constexpr block(block && other) noexcept = default;

	/**	\fn block(std::nullptr_t) noexcept
	 *	\brief Creates a block with no function.
	 *	\details Can be used as a block to signal the end of execution.
	 */
	constexpr block(std::nullptr_t) noexcept : ptr(nullptr) {}

	/**	\fn ~block() noexcept
	 *	\brief Default.
	 */
	~block() noexcept = default;

	/**	\fn operator==(block const& other) const noexcept
	 *	\brief Checks equality of two blocks.
	 *	\param other The block to compare with.
	 *	\returns Returns true if other and this point to the same function.
	 */
	constexpr bool operator==(block const& other) const noexcept {
		return ptr == other.ptr;
	}

	/**	\fn operator!=(block const& other) const noexcept
	 *	\brief Checks inequality of two blocks.
	 *	\param other The block to compare with.
	 *	\returns Returns false if other and this point to the same function.
	 */
	constexpr bool operator!=(block const& other) const noexcept {
		return ptr != other.ptr;
	}

	/**	\fn operator==(std::nullptr_t) const noexcept
	 *	\brief Checks that the block has no function.
	 *	\returns Returns true if ptr is nullptr.
	 */
	constexpr bool operator==(std::nullptr_t) const noexcept {
		return ptr == nullptr;
	}

	/**	\fn operator!=(std::nullptr_t) const noexcept
	 *	\brief Checks that the block has a function.
	 *	\returns Returns false if ptr is nullptr.
	 */
	constexpr bool operator!=(std::nullptr_t) const noexcept {
		return ptr != nullptr;
	}

	/**	\fn operator!() const noexcept
	 *	\brief Checks if the block does not have a function.
	 *	\returns Returns true if ptr is nullptr.
	 */
	constexpr bool operator!() const noexcept {
		return ptr == nullptr;
	}

	/**	\fn operator bool() const noexcept
	 *	\brief Implicit conversion to bool.
	 *	\returns Returns false if ptr is nullptr.
	 */
	constexpr operator bool() const noexcept {
		return ptr != nullptr;
	}

	/**	\fn operator=(block const& other) noexcept
	 *	\brief The member ptr of this is set to the value of other.
	 *	\param other The block to copy.
	 *	\returns *this
	 */
	constexpr block &operator=(block const& other) noexcept {
		ptr = other.ptr;
		return *this;
	}

	/**	\fn operator=(block && other) noexcept
	 *	\brief The member ptr of this is set to the value of other.
	 *	\param other The block to move to this.
	 *	\returns *this
	 */
	constexpr block &operator=(block && other) noexcept {
		ptr = other.ptr;
		return *this;
	}

	/**	\fn operator()(state_type &state) const
	 *	\brief Runs the code with a given state.
	 *	\pre ptr != nullptr
	 *	\param state The execution state.
	 *	\details Calls the function.
	 *	\return Returns the next block to execute.
	 */
	block operator()(state_type &state) const {
		SYNAFIS_ASSERT(ptr != nullptr);
		return (*ptr)(state);
	}
};

}

#endif
