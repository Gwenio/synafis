
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

#ifndef SYNAFIS_GC_IMPL_HPP
#define SYNAFIS_GC_IMPL_HPP
#pragma once

/**	\file gc/impl.hpp
 *	\brief Defines the garbage collector implementation.
 */

namespace gc {

/**	\class collector
 *	\brief The main class implementing the garbage collector.
 *	\todo This is a stub waiting to be implemented.
 */
class collector {
public:
	/**	\fn collector() noexcept
	 *	\brief Sets up the collector as much as possible in a constexpr function.
	 *	\note The reason for constexpr is to ensure a certain amount of setup
	 *	\note will be done regardless of the order global objects are initialized in.
	 */
	constexpr collector() noexcept;

	/**	\fn ~collector() noexcept
	 *	\brief Sets up the collector as much as possible in a constexpr function.
	 */
	~collector() noexcept;

	/**	\fn init()
	 *	\brief Finishes setting up the collector.
	 */
	void init();

	/**	\fn lock()
	 *	\brief Locks against collection.
	 *	\details Called by the program when entering a section in which the collector must not run.
	 *	\todo In particular this call starts the thread that preforms collection.
	 */
	void lock();

	/**	\fn unlock()
	 *	\brief Allows collection again.
	 *	\details Called by the program when leaving a section in which the collector must not run.
	 *	\note If the collector needs to run, this call will block until after it runs.
	 */
	void unlock();

	/**	\fn get_soft_ptr(void *ptr)
	 *	\brief Gets the soft pointer associated with an object.
	 *	\param ptr A pointer to the object to get the soft pointer data for.
	 *	\returns Returns a pointer to the soft pointer data.
	 *	\throws Throws std::bad_alloc if a soft_ptr::data needed to be allocated but memory was lacking.
	 *	\details Gets the existing data if there is one or creates it if there is not.
	 */
	soft_ptr::data *get_soft_ptr(void *ptr);

	/**	\fn free_soft_ptr(soft_ptr::data *ptr)
	 *	\brief Frees the soft pointer data.
	 *	\param ptr The data to deallocate.
	 *	\pre The object the soft pointer refers to must no longer exist or a replacement has been set in next.
	 *	\pre Also the reference count must be zero.
	 *	\post The data object is no longer valid.
	 *	\details Deallocates the memory if needed.
	 */
	void free_soft_ptr(soft_ptr::data *ptr);

	/**	\fn base_ptr() noexcept
	 *	\brief Gets the originally allocated address.
	 *	\param ptr The pointer to get a base address for.
	 *	\returns Returns the address originally allocated for the object ptr points to a location within.
	 */
	void *base_ptr(void *ptr) noexcept;
};

}

#endif
