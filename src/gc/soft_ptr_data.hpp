
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

#ifndef SYNAFIS_GC_SOFT_PTR_DATA_HPP
#define SYNAFIS_GC_SOFT_PTR_DATA_HPP
#pragma once

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

/**	\file src/gc/soft_ptr_data.hpp
 *	\brief Defines the implementation for parts of the GC API.
 */

#include <atomic>

namespace gc {

class soft_ptr::data
{
public:
	/**	\var ptr
	 *	\brief A pointer to the object weakly referenced.
	 */
	void *ptr;

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

	/**	\fn data(void *obj) noexcept
	 *	\brief Creates a new data object.
	 *	\param obj The address of the object the data points to.
	 *	\details The reference count starts at zero so we can increment it with
	 *	\details copy when returned by collector::get_soft_ptr just as we would
	 *	\details for a pre-existing data.
	 */
	constexpr data(void *obj) noexcept : ptr(obj), next(nullptr), count(0) {}

	/**	\fn ~data() noexcept
	 *	\brief Default.
	 */
	~data() noexcept = default;

	/**	\fn operator=(std::nullptr_t) noexcept
	 *	\brief Sets ptr to nullptr.
	 *	\returns *this
	 *	\post The data is stale, and will be deallocated when the reference count reaches zero.
	 */
	constexpr data &operator=(std::nullptr_t) noexcept
	{
		ptr = nullptr;
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
		return *this;
	}
};

}

#endif
