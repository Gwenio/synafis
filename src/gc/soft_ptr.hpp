
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

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

#ifndef SYNAFIS_GC_SOFT_PTR_HPP
#define SYNAFIS_GC_SOFT_PTR_HPP
#pragma once

/**	\file src/gc/soft_ptr.hpp
 *	\brief Defines the type for soft garbage collector pointers.
 */

namespace gc {

class hard_ptr;

/**	\class soft_ptr
 *	\brief Type for managing soft or weak pointers.
 */
class soft_ptr {
	friend soft_ptr;
	friend hard_ptr;
	friend unit_test::tester<soft_ptr>;
public:
	/**	\class data
	 *	\brief Opaque type for soft pointer data.
	 */
	class data;
private:
	/**	\var ptr
	 *	\brief Pointer to the data for the soft_ptr.
	 */
	data *ptr;

	/**	\fn get_soft(hard_ptr const&other)
	 *	\brief Gets the soft pointer data for a hard pointer.
	 *	\param other The hard_ptr to get the data for.
	 *	\returns Returns the associated data.
	 */
	static data *get_soft(hard_ptr const&other);

	/**	\fn copy(data *other) noexcept
	 *	\brief Increase the refernce count on the data and returns it.
	 *	\param other The data to increase the reference count of.
	 *	\returns Returns the data.
	 *	\details If applicable, cleans up a stale data as update() would.
	 *	\note Calls free() on 'other' if returning a different value.
	 */
	static data *copy(data *other) noexcept;

	/**	\fn update(data *old) noexcept
	 *	\brief Cleans up stale soft pointers.
	 *	\param old The previous data.
	 *	\returns Returns the data, a new data if the object was merged, or
	 *	\returns nullptr if the object has been deallocated.
	 *	\details When immutable objects are merged, this allows
	 *	\details the soft pointers to eventually merge as well.
	 *	\details It also cleans up leftover soft pointers for
	 *	\details objects that have been deallocated.
	 *	\note Calls free() on 'old' if returning a different value.
	 */
	static data *update(data *old) noexcept;

	/**	\fn free(data *other) noexcept
	 *	\brief Reduces the reference count of the data.
	 *	\param other The data to reduce the reference count of.
	 *	\details If the reference count is reduced to zero and the
	 *	\details associated object has been deallocated by the collector,
	 *	\details then we deallocate the data.
	 */
	static void free(data *other) noexcept;
public:
	/**	\fn soft_ptr()
	 *	\brief Initialize with a null pointer.
	 */
	constexpr soft_ptr() noexcept : ptr(nullptr) {}

	/**	\fn soft_ptr(std::nullptr_t)
	 *	\brief Initialize with a null pointer and no type.
	 */
	constexpr soft_ptr(std::nullptr_t) noexcept : soft_ptr() {}

	/**	\fn soft_ptr(soft_ptr const& other)
	 *	\brief Initialize a copy of another soft_ptr.
	 *	\param other The soft_ptr to copy.
	 *	\see copy
	 */
	soft_ptr(soft_ptr const& other);

	/**	\fn soft_ptr(soft_ptr && other)
	 *	\brief Initialize by moving another soft_ptr's content to this one.
	 *	\param other The soft_ptr to move to this.
	 *	\post other.data == nullptr
	 */
	soft_ptr(soft_ptr && other);

	/**	\fn soft_ptr(hard_ptr const& other)
	 *	\brief Initialize by getting the soft_ptr for a hard_ptr.
	 *	\param other The hard_ptr to get a soft_ptr for.
	 *	\see get_hard
	 */
	soft_ptr(hard_ptr const& other);

	/**	\fn ~soft_ptr()
	 *	\brief If holding a soft reference pointer, free it.
	 *	\see free
	 */
	~soft_ptr();

	/**	\fn operator=(std::nullptr_t) noexcept
	 *	\brief If ptr != nullptr, free it and set it to nullptr.
	 *	\returns *this
	 *	\see free
	 */
	soft_ptr &operator=(std::nullptr_t) noexcept;

	/**	\fn operator=(soft_ptr const& other) noexcept
	 *	\brief Copy the data from another soft_ptr.
	 *	\param other The soft_ptr to copy.
	 *	\returns *this
	 *	\post other == *this
	 *	\see refresh
	 *	\see copy
	 */
	soft_ptr &operator=(soft_ptr const& other) noexcept;

	/**	\fn operator=(soft_ptr && other) noexcept
	 *	\brief Move the data from another soft_ptr.
	 *	\param other The soft_ptr to move the data from.
	 *	\returns *this
	 *	\post other.ptr == nullptr unless other is this.
	 *	\see refresh
	 *	\see update
	 */
	soft_ptr &operator=(soft_ptr && other) noexcept;

	/**	\fn soft_ptr &operator=(hard_ptr const& other) noexcept
	 *	\brief Set this to the soft_ptr of other.
	 *	\param other The hard_ptr to get a soft_ptr for.
	 *	\detail We avoid getting a soft_ptr if other is null and set this to nullptr instead.
	 *	\see get_soft
	 */
	soft_ptr &operator=(hard_ptr const& other) noexcept;

	/**	\fn operator bool() const noexcept
	 *	\brief Converts to bool.
	 *	\returns Returns true if ptr != nullptr.
	 */
	constexpr operator bool() const noexcept {
		return ptr != nullptr;
	}

	/**	\fn operator!() const noexcept
	 *	\brief Converts to bool.
	 *	\returns Returns true if ptr == nullptr.
	 */
	constexpr bool operator!() const noexcept {
		return ptr == nullptr;
	}

	/**	\fn operator==(soft_ptr const& other) const noexcept
	 *	\brief Compares soft pointers for traits::compare.
	 *	\returns Returns true if the objects referred to will always be equal.
	 */
	bool operator==(soft_ptr const& other) const noexcept;

	/**	\fn refresh() noexcept
	 *	\brief Updates ptr with update().
	 *	\note May safely be called when an object containing the soft_ptr
	 *	\note is having its pointers remapped by the collector.
	 *	\see update
	 */
	void refresh() noexcept;
};

}

#endif
