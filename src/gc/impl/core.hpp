
/*
ISC License (ISC)

Copyright 2018-2019 Adam Armstrong

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

/**	\file src/gc/impl/core.hpp
 *	\brief Defines the garbage collector implementation.
 *	\ingroup gc_impl
 */

#include "source.hpp"
#include "../identity.hpp"

#ifndef SYNAFIS_GC_CORE_HPP
#define SYNAFIS_GC_CORE_HPP
#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <vector>

namespace gc {

/**	\class collector
 *	\brief The main class implementing the garbage collector.
 *	\ingroup gc_impl
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
	/**	\class iallocator
	 *	\brief Extends the interface for allocators for internal use.
	 */
	class iallocator : public identity::iallocator
	{
	protected:
		/**	\fn iallocator() noexcept
		 *	\brief Default.
		 */
		constexpr iallocator() noexcept = default;

		/**	\fn insert_source(isource &src, bool trav) noexcept
		 *	\brief Adds a source to the collector to track.
		 *	\param src The source to track.
		 *	\param trav Indicates if the source is traversable.
		 */
		static void insert_source(isource &src, bool trav) noexcept
		{
			collector::singleton().insert_source(src, trav);
		}

		/**	\fn erase_sources(Iter const begin, Iter const end, bool trav) noexcept
		 *	\brief Stops tracking as set of sources.
		 *	\tparam Iter The type of iterator. Must be a forward iterator.
		 *	\param begin The iterator of the first source to stop tracking.
		 *	\param end The iterator past the last source to stop tracking.
		 *	\param trav Indicates if the sources are traversable.
		 */
		template<typename Iter>
		static void erase_sources(Iter const begin, Iter const end, bool trav) noexcept
		{
			std::lock_guard<std::mutex> l{collector::singleton().mtx};
			erase_sources(begin, end, trav, std::defer_lock);
		}

		/**	\fn erase_sources(Iter const begin, Iter const end, bool trav, std::defer_lock_t) noexcept
		 *	\brief Stops tracking as set of sources.
		 *	\tparam Iter The type of iterator. Must be a forward iterator.
		 *	\param begin The iterator of the first source to stop tracking.
		 *	\param end The iterator past the last source to stop tracking.
		 *	\param trav Indicates if the sources are traversable.
		 *	\pre Only call in response to a shrink request.
		 */
		template<typename Iter>
		static void erase_sources(
			Iter const begin, Iter const end, bool trav, std::defer_lock_t) noexcept
		{
			for (auto cur = begin; cur != end; cur++) {
				collector::singleton().erase_source(*cur, trav);
			}
		}

	public:
		/**	\fn ~iallocator()
		 *	\brief Default.
		 */
		virtual ~iallocator() noexcept = default;

		/**	\fn shrink(std::size_t goal) noexcept
		 *	\brief Causes the allocator to try and free unneeded memory.
		 *	\param goal A hint about how much memory to try and free.
		 *	\returns Returns the units of memory freed.
		 *	\details The measures of memory are not exact, just hints. The hints are a count of
		 *	\details units that is approximately (vmem::page_size() * config::max_pool).
		 *	\details Will be called with a goal of zero to notify that it is a good time to shrink
		 *	\details if the allocator has lots of free space.
		 *	\note Allocators need to keep some free space available for allocation.
		 */
		virtual std::size_t shrink(std::size_t goal) noexcept = 0;
	};
	//!	\cond friends
	friend iallocator;
	//!	\endcond

	/**	\typedef duration
	 *	\brief The type used to describe the length of a time period.
	 */
	using duration = std::chrono::steady_clock::duration;

	/**	\typedef alloc_ptr
	 *	\brief The type for stored allocators.
	 */
	using alloc_ptr = std::unique_ptr<iallocator>;

	/**	\struct root_type
	 *	\brief The type to store root objects.
	 *	\see roots
	 */
	struct root_type
	{
		/**	\var tcb
		 *	\brief The callback for traversing pointers in the object.
		 */
		traverse_cb const tcb;

		/**	\var rcb
		 *	\brief The callback for updating pointers in the object.
		 */
		root_cb const rcb;
	};

private:
	/**	\var mtx
	 *	\brief The mutex for the collector lock.
	 *	\note std::shared_mutex is not used as it cannot be counted on to be writer favored.
	 *	\todo This mutex is used for all synchronization for now. Later investigate if some
	 *	\todo parts of the interface could use a separate one.
	 */
	std::mutex mutable mtx;

	/**	\var alive
	 *	\brief Used to signal the worker to shutdown.
	 */
	std::atomic<bool> alive;

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

	/**	\var requests
	 *	\brief The number of failures to allocate memory since a collection cycle.
	 *	\note Incremented when wait_impl() is called.
	 */
	std::size_t requests;

	/**	\var period
	 *	\brief The time between unforced collection cycles.
	 */
	duration period;

	/**	\var sources
	 *	\brief Bookkeeping for sources of allocation.
	 */
	std::map<void *, isource *> sources;

	/**	\var traversable
	 *	\brief The traversable sources.
	 */
	std::set<isource *> traversable;

	/**	\var allocators
	 *	\brief Tracks allocators for shrink requests.
	 */
	std::vector<alloc_ptr> allocators;

	/**	\var roots
	 *	\brief Tracks root objects.
	 *	\note Tracked roots should be addresses not allocated by the GC.
	 */
	std::map<void *, root_type> roots;

	/**	\fn collector() noexcept
	 *	\brief Prepares most of the collector.
	 */
	collector() noexcept;

	/**	\fn singleton() noexcept
	 *	\brief The singleton of type collector.
	 *	\return Returns a reference the the singleton of the collector type.
	 */
	static collector &singleton() noexcept;

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
	void wait_impl()
	{
		std::unique_lock<std::mutex> l{mtx};
		wait_impl(l);
	}

	/**	\fn wait_impl(std::unique_lock<std::mutex> &l)
	 *	\brief Version of wait_impl() for internal use.
	 *	\param l The the lock.
	 *	\pre The lock l must own the lock on mtx.
	 *	\post The lock l will own the lock on mtx.
	 *	\see wait()
	 */
	void wait_impl(std::unique_lock<std::mutex> &l);

	/**	\fn collect_impl() noexcept
	 *	\see collect()
	 */
	void collect_impl() noexcept
	{
		{
			std::lock_guard<std::mutex> l{mtx};
			flag = false;
		}
		writer.notify_one();
	}

	/**	\fn register_root_impl(void *obj, traverse_cb tcb, root_cb rcb)
	 *	\param obj A pointer to the root object being registered.
	 *	\param tcb The callback for traversing pointers in the object.
	 *	\param rcb The callback for updating pointers in the object.
	 *	\see register_root(void *obj, traverse_cb tcb, root_cb rcb)
	 */
	void register_root_impl(void *obj, traverse_cb tcb, root_cb rcb);

	/**	\fn unregister_root_impl(void *obj) noexcept
	 *	\param obj A pointer to the root object to unregister.
	 *	\see unregister_root(void *obj) noexcept
	 */
	void unregister_root_impl(void *obj) noexcept;

	/**	\fn find_source_impl(void *ptr) const noexcept
	 *	\brief Gets the source of a pointer.
	 *	\param ptr The pointer to get a base address for.
	 *	\returns Returns the source of ptr or nullptr if none.
	 *	\pre The caller must own the lock on mtx.
	 */
	isource *find_source_impl(void *ptr) const noexcept;

	/**	\fn find_source_wrap(void *ptr) const noexcept
	 *	\brief A wrapper for find_source_impl() that locks mtx.
	 *	\param ptr The pointer to get a base address for.
	 *	\returns Returns the source of ptr or nullptr if none.
	 */
	isource *find_source_wrap(void *ptr) const noexcept
	{
		std::lock_guard<std::mutex> l{mtx};
		return find_source_impl(ptr);
	}

	/**	\fn base_ptr_impl(void *ptr) const noexcept
	 *	\param ptr The pointer to get the base address for.
	 *	\returns Returns the base pointer for ptr.
	 *	\see base_ptr(void *ptr) noexcept
	 */
	void *base_ptr_impl(void *ptr) const noexcept;

	/**	\fn get_type_impl(void *ptr) const noexcept
	 *	\param ptr The pointer to the type for.
	 *	\returns Returns the identity for ptr.
	 *	\see get_type(void *ptr) noexcept
	 */
	identity const *get_type_impl(void *ptr) const noexcept;

	/**	\fn set_period_impl(duration value) noexcept
	 *	\brief Sets period.
	 *	\param value The new value for period.
	 */
	void set_period_impl(duration value) noexcept
	{
		std::lock_guard<std::mutex> l{mtx};
		period = value;
	}

	/**	\fn insert_source(isource &src, bool trav) noexcept
	 *	\brief Adds a source to the collector to track.
	 *	\param src The source to track.
	 *	\param trav Indicates if the source is traversable.
	 */
	void insert_source(isource &src, bool trav) noexcept;

	/**	\fn erase_source(isource const &src, bool trav) noexcept
	 *	\brief Stops tracking a source.
	 *	\param src The source to remove.
	 *	\param trav Indicates if the source is traversable.
	 */
	void erase_source(isource const &src, bool trav) noexcept;

	/**	\fn insert_alloc_impl(alloc_ptr &&alloc) noexcept
	 *	\brief Takes ownership of an allocator.
	 *	\param alloc The source to take ownership of.
	 *	\returns Returns a pointer to the inserted allocator.
	 */
	iallocator *insert_alloc_impl(alloc_ptr &&alloc) noexcept;

	/**	\fn work() noexcept
	 *	\brief Defines the job of worker.
	 */
	void work() noexcept;

	/**	\fn enumerate(void *data, void *ptr) noexcept
	 *	\brief Marks the objects indicated by ptr.
	 *	\param data A pointer to a collector.
	 *	\param ptr A pointer to the object to mark.
	 */
	static void enumerate(void *data, void *ptr) noexcept;

	/**	\fn mark() noexcept
	 *	\brief Preforms the marking phase of a collection cycle.
	 */
	void mark() noexcept;

	/**	\fn sweep() noexcept
	 *	\brief Preforms the sweep phase of a collection cycle.
	 */
	void sweep() noexcept;

	/**	\fn shrink() noexcept
	 *	\brief Notifies allocators to release memory.
	 */
	void shrink() noexcept;

	/**	\fn wait_read() const
	 *	\brief Checks if a thread other than worker should continue to wait.
	 *	\returns Returns true if the thread may stop waiting.
	 *	\throws Throws std::runtime_error if the collector's destructor has been called.
	 */
	bool wait_read() const;

public:
	/**	\fn ~collector() noexcept
	 *	\brief Destructor.
	 *	\pre managed.empty() && unmanaged.empty()
	 */
	~collector() noexcept;

	/**	\fn init()
	 *	\brief Finishes setting up the collector.
	 */
	static void init() { singleton().init_impl(); }

	/**	\fn lock()
	 *	\brief Locks against collection.
	 *	\details Called by the program when entering a section in which the collector must not run.
	 *	\note If the collector needs to run, this call will block until after it runs.
	 */
	static void lock() { singleton().lock_impl(); }

	/**	\fn unlock()
	 *	\brief Allows collection again.
	 *	\details Called by the program when leaving a section in which the collector must not run.
	 */
	static void unlock() { singleton().unlock_impl(); }

	/**	\fn wait()
	 *	\brief Called when insufficient memory was available.
	 *	\details Signals that a collection cycle is needed.
	 *	\details Does not return until a cycle has run.
	 */
	static void wait() { singleton().wait_impl(); }

	/**	\fn collect() noexcept
	 *	\brief Causes a collection cycle to run as soon as possible.
	 */
	static void collect() noexcept { singleton().collect_impl(); }

	/**	\fn base_ptr(void *ptr) noexcept
	 *	\brief Gets the originally allocated address.
	 *	\param ptr The pointer to get the base address for.
	 *	\returns Returns the address originally allocated for the object ptr points to a
	 *	\returns location within. Or nullptr if not allocated by a registered source.
	 */
	static void *base_ptr(void *ptr) noexcept { return singleton().base_ptr_impl(ptr); }

	/**	\fn get_type(void *ptr) noexcept
	 *	\brief Gets the originally allocated address.
	 *	\param ptr The pointer to get the identity for.
	 *	\returns Returns the identity of an object or nullptr if not from a registered source.
	 */
	static identity const *get_type(void *ptr) noexcept { return singleton().get_type_impl(ptr); }

	/**	\fn register_root(void *obj, traverse_cb tcb, root_cb rcb)
	 *	\brief Registers a root object with the collector.
	 *	\param obj A pointer to the root object being registered.
	 *	\param tcb The callback for traversing pointers in the object.
	 *	\param rcb The callback for updating pointers in the object.
	 *	\pre obj != nullptr && find_source(obj) == nullptr
	 */
	static void register_root(void *obj, traverse_cb tcb, root_cb rcb)
	{
		return singleton().register_root_impl(obj, tcb, rcb);
	}

	/**	\fn unregister_root(void *obj) noexcept
	 *	\brief Unregisters a root object.
	 *	\param obj A pointer to the root object to unregister.
	 */
	static void unregister_root(void *obj) noexcept { singleton().unregister_root_impl(obj); }

	/**	\fn set_period(duration value) noexcept
	 *	\brief Sets the period for singleton().
	 *	\param value The new value for period.
	 */
	static void set_period(duration value) noexcept { singleton().set_period_impl(value); }

	/**	\fn insert_alloc(alloc_ptr &&alloc) noexcept
	 *	\brief Takes ownership of an allocator.
	 *	\param alloc The source to take ownership of.
	 *	\returns Returns a pointer to the inserted allocator.
	 */
	static iallocator *insert_alloc(alloc_ptr &&alloc) noexcept
	{
		return singleton().insert_alloc_impl(std::forward<alloc_ptr>(alloc));
	}

	/**	\fn find_source(void *ptr) noexcept
	 *	\brief Finds the source an address was allocated from.
	 *	\param ptr The pointer to get a base address for.
	 *	\returns Returns the source of ptr or nullptr if none.
	 */
	static isource *find_source(void *ptr) noexcept { return singleton().find_source_wrap(ptr); }
};

}

#endif
