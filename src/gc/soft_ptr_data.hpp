
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

#ifndef SYNAFIS_GC_SOURCE_HPP
#include "source.hpp"
#endif

#ifndef SYNAFIS_GC_SOFT_PTR_DATA_HPP
#define SYNAFIS_GC_SOFT_PTR_DATA_HPP
#pragma once

/**	\file src/gc/soft_ptr_data.hpp
 *	\brief Defines the implementation for parts of the GC API.
 */

#ifndef SYNAFIS_STDINC_ATOMIC
#include <atomic>
#define SYNAFIS_STDINC_ATOMIC
#endif

#ifndef SYNAFIS_STDINC_MEMORY
#include <memory>
#define SYNAFIS_STDINC_MEMORY
#endif

#ifndef SYNAFIS_STDINC_MUTEX
#include <mutex>
#define SYNAFIS_STDINC_MUTEX
#endif

#ifndef SYNAFIS_STDINC_VECTOR
#include <vector>
#define SYNAFIS_STDINC_VECTOR
#endif

namespace gc {

class soft_ptr::data
{
	//!	\cond friends
	friend soft_ptr;
	//!	\endcond
private:
	/**	\var mtx
	 *	\brief The mutex for static member records.
	 */
	static std::mutex mtx;

	/**	\var stale
	 *	\brief Tracks the soft pointers waiting for deallocation.
	 *	\invariant Should be kept sorted by address pointed to.
	 */
	static std::vector<data *> stale;

	/**	\fn insert_stale(data *ptr) noexcept
	 *	\brief Puts ptr in stale.
	 *	\param ptr The object that is now stale.
	 *	\details De-increments ptr->count, and frees it without inserting if it is zero.
	 */
	static void insert_stale(data *ptr) noexcept;

	/**	\var next
	 *	\brief A pointer to another data that takes precedence over this.
	 *	\details When immutable objects are merged, soft pointers to those
	 *	\details objects have next set to the data of the soft pointer for
	 *	\details the remaining object.
	 *	\todo It is likely that either next does not need to be atomic or
	 *	\todo that ptr does. But more of the collector interals need to be
	 *	\todo implemented to tell for sure.
	 */
	std::atomic<data *> next;

	/**	\var count
	 *	\brief Number of times the data is referenced.
	 */
	std::atomic<std::size_t> count;

	/**	\var ptr
	 *	\brief A pointer to the object weakly referenced.
	 *	\warning Only modify from the collector's worker.
	 */
	void *ptr;

	/**	\fn data(void *p) noexcept
	 *	\brief Creates a new data object.
	 *	\param p The pointer to the object associated with the data.
	 *	\details The reference count starts at one so the data will exist at least until it is
	 *	\details marked as stale.
	 */
	constexpr data(void *p) noexcept : next(nullptr), count(1), ptr(p) {}

	/**	\fn ~data() noexcept
	 *	\brief Default.
	 */
	~data() noexcept = default;

public:
	/**	\fn operator=(std::nullptr_t) noexcept
	 *	\brief Sets ptr to nullptr.
	 *	\returns *this
	 *	\post The data is stale, and will be deallocated when the reference count reaches zero.
	 */
	data &operator=(std::nullptr_t) noexcept
	{
		ptr = nullptr;
		insert_stale(this);
		return *this;
	}

	/**	\fn operator=(data *other) noexcept
	 *	\brief Sets next to other.
	 *	\param other The value to set next to.
	 *	\returns *this
	 *	\post The data is stale, and will be deallocated when the reference count reaches zero.
	 */
	data &operator=(data *other) noexcept
	{
		SYNAFIS_ASSERT(other != nullptr);
		SYNAFIS_ASSERT(other != this);
		SYNAFIS_ASSERT(other->next == nullptr);
		next.store(other);
		insert_stale(this);
		return *this;
	}

	/**	\fn get() const noexcept
	 *	\brief Gets the value of ptr.
	 *	\returns ptr
	 */
	void *get() const noexcept { return ptr; }

	/**	\fn create(void *ptr) noexcept
	 *	\brief Creates a new soft_ptr::data.
	 *	\param ptr The pointer to the object associated with the data.
	 *	\returns Returns a pointer to the new soft pointer data.
	 */
	static data *create(void *ptr) noexcept;

	/**	\fn get_soft_ptr(void *ptr) noexcept
	 *	\brief Gets the soft pointer associated with an object.
	 *	\param ptr A pointer to the object to get the soft pointer data for.
	 *	\returns Returns a pointer to the soft pointer data.
	 *	\details Gets the existing data if there is one or creates it if there is not.
	 */
	static soft_ptr::data *get_soft_ptr(void *ptr) noexcept;

	/**	\fn free_soft_ptr(soft_ptr::data *ptr) noexcept
	 *	\brief Frees the soft pointer data.
	 *	\param ptr The data to deallocate.
	 *	\pre ptr->count.load() == 0
	 *	\post The data object is no longer valid.
	 *	\details Deallocates the memory.
	 */
	static void free_soft_ptr(soft_ptr::data *ptr) noexcept;
};

}

#endif
