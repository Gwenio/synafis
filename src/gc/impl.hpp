
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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

/**	\file src/gc/impl.hpp
 *	\brief Defines the garbage collector implementation.
 */

namespace gc {

class collector;

/**	\class collector
 *	\brief The main class implementing the garbage collector.
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

	/**	\typedef duration
	 *	\brief The type used to describe the length of a time period.
	 */
	using duration = std::chrono::steady_clock::duration;

private:
	/**	\var mtx
	 *	\brief The mutex for the collector lock.
	 *	\note std::shared_mutex is not used as it cannot be counted on to be writer favored.
	 */
	std::mutex mtx;

	/**	\var readers
	 *	\brief The condition variable for lock() requests to wait with.
	 *	\details When flag is false lock() will wait with readers until flag is true.
	 */
	std::condition_variable readers;

	/**	\var writer
	 *	\brief The condition variable for the collector thread to wait on.
	 */
	std::condition_variable writer;

	/**	\var flag
	 *	\brief Indicates if the collector is waiting to run a mark and sweep cycle.
	 *	\details When true lock() will not wait for a mark and sweep cycle.
	 *	\details Set to false when a cycle is pending.
	 */
	bool flag;

	/**	\var count
	 *	\brief The number of active readers.
	 *	\details Collection must wait for this number to be zero.
	 */
	std::size_t count;

	/**	\var worker
	 *	\brief The worker thread of the collector.
	 */
	std::thread worker;

	/**	\var alive
	 *	\brief Used to signal the worker to shutdown.
	 */
	std::atomic_flag alive;

	/**	\var period
	 *	\brief The time between unforced collection cycles.
	 */
	duration period;

	/**	\fn collector() noexcept
	 *	\brief Prepares most of the collector.
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

	/**	\fn wait_impl()
	 *	\see wait()
	 */
	void wait_impl();

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

	/**	\fn set_period_impl(duration value) noexcept
	 *	\brief Sets period.
	 *	\param value The new value for period.
	 */
	void set_period_impl(duration value) noexcept
	{
		std::lock_guard<std::mutex> l{mtx};
		period = value;
	}

	/**	\fn work() noexcept
	 *	\brief Defines the job of worker.
	 */
	void work() noexcept;

public:
	/**	\fn ~collector() noexcept
	 *	\brief Destructor.
	 */
	~collector() noexcept;

	/**	\fn init()
	 *	\brief Finishes setting up the collector.
	 *	\todo In particular this call starts the thread that preforms collection.
	 */
	static void init() { singleton.init_impl(); }

	/**	\fn lock()
	 *	\brief Locks against collection.
	 *	\details Called by the program when entering a section in which the collector must not run.
	 *	\note If the collector needs to run, this call will block until after it runs.
	 */
	static void lock() { singleton.lock_impl(); }

	/**	\fn unlock()
	 *	\brief Allows collection again.
	 *	\details Called by the program when leaving a section in which the collector must not run.
	 */
	static void unlock() { singleton.unlock_impl(); }

	/**	\fn wait()
	 *	\brief Called when insufficent memory was available.
	 *	\details Signals that a collection cycle is needed.
	 *	\details Does not return until a cycle has run.
	 */
	static void wait() { singleton.wait_impl(); }

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

	/**	\fn set_period(duration value) noexcept
	 *	\brief Sets the period for singleton.
	 *	\param value The new value for period.
	 */
	static void set_period(duration value) noexcept { singleton.set_period_impl(value); }
};

}

#endif
