
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

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

/**	\file src/gc/impl.hpp
 *	\brief Defines the garbage collector implementation.
 */

namespace gc {

/**	\class collector
 *	\brief The main class implementing the garbage collector.
 *	\todo This is a stub waiting to be implemented.
 */
class collector
{
	//!	\cond friends
	friend unit_test::tester<collector>;
	//!	\endcond

	/**	\fn collector(collector const &)
	 *	\brief Deleted.
	 */
	collector(collector const &) = delete;

	/**	\fn collector(collector &&)
	 *	\brief Deleted.
	 */
	collector(collector &&) = delete;

public:
	/**	\class region
	 *	\brief Represents regions of memory that may be allocated.
	 *	\details Used for bookkeeping that needs to know where an object was allocated from.
	 */
	class region
	{
		//!	\cond friends
		friend unit_test::tester<collector>;
		//!	\endcond
	private:
	public:
	};

private:
	/**	\fn collector() noexcept
	 *	\brief Sets most of the collector.
	 *	\note Needs to be independent of the order global objects are constructed in.
	 */
	collector() noexcept;

	/**	\var singleton
	 *	\brief The singleton of type collector.
	 */
	static collector singleton;

	/**	\fn init_impl()
	 *	\see init()
	 */
	void init_impl();

	/**	\fn lock_impl()
	 *	\see lock()
	 */
	void lock_impl();

	/**	\fn unlock_impl()
	 *	\see unlock()
	 */
	void unlock_impl();

	/**	\fn get_soft_ptr_impl(void *ptr)
	 *	\param ptr A pointer to the object to get the soft pointer data for.
	 *	\see get_soft_ptr(void *ptr)
	 */
	soft_ptr::data *get_soft_ptr_impl(void *ptr);

	/**	\fn free_soft_ptr_impl(soft_ptr::data *ptr)
	 *	\param ptr The data to deallocate.
	 *	\see free_soft_ptr(soft_ptr::data *ptr)
	 */
	void free_soft_ptr_impl(soft_ptr::data *ptr);

	/**	\fn base_ptr_impl(void *ptr) noexcept
	 *	\param ptr The pointer to get a base address for.
	 *	\see base_ptr(void *ptr) noexcept
	 */
	void *base_ptr_impl(void *ptr) noexcept;

public:
	/**	\fn ~collector() noexcept
	 *	\brief Destructor.
	 */
	~collector() noexcept;

	/**	\fn init()
	 *	\brief Finishes setting up the collector.
	 */
	static void init() { singleton.init_impl(); }

	/**	\fn lock()
	 *	\brief Locks against collection.
	 *	\details Called by the program when entering a section in which the collector must not run.
	 *	\todo In particular this call starts the thread that preforms collection.
	 */
	static void lock() { singleton.lock_impl(); }

	/**	\fn unlock()
	 *	\brief Allows collection again.
	 *	\details Called by the program when leaving a section in which the collector must not run.
	 *	\note If the collector needs to run, this call will block until after it runs.
	 */
	static void unlock() { singleton.unlock_impl(); }

	/**	\fn get_soft_ptr(void *ptr)
	 *	\brief Gets the soft pointer associated with an object.
	 *	\param ptr A pointer to the object to get the soft pointer data for.
	 *	\returns Returns a pointer to the soft pointer data.
	 *	\throws Throws std::bad_alloc if a soft_ptr::data needed to be allocated but memory
	 *	\throws was lacking.
	 *	\details Gets the existing data if there is one or creates it if there is not.
	 */
	static soft_ptr::data *get_soft_ptr(void *ptr) { return singleton.get_soft_ptr_impl(ptr); }

	/**	\fn free_soft_ptr(soft_ptr::data *ptr)
	 *	\brief Frees the soft pointer data.
	 *	\param ptr The data to deallocate.
	 *	\pre The object the soft pointer refers to must no longer exist or a replacement has
	 *	\pre been set in next.
	 *	\pre Also the reference count must be zero.
	 *	\post The data object is no longer valid.
	 *	\details Deallocates the memory if needed.
	 */
	static void free_soft_ptr(soft_ptr::data *ptr) { singleton.free_soft_ptr_impl(ptr); }

	/**	\fn base_ptr(void *ptr) noexcept
	 *	\brief Gets the originally allocated address.
	 *	\param ptr The pointer to get a base address for.
	 *	\returns Returns the address originally allocated for the object ptr points to a
	 *	\returns location within.
	 */
	static void *base_ptr(void *ptr) noexcept { return singleton.base_ptr_impl(ptr); }
};

}

#endif
