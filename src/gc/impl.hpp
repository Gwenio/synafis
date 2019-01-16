
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

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

#ifndef SYNAFIS_GC_SOURCE_HPP
#include "source.hpp"
#endif

#ifndef SYNAFIS_GC_IMPL_HPP
#define SYNAFIS_GC_IMPL_HPP
#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

/**	\file src/gc/impl.hpp
 *	\brief Defines the garbage collector implementation.
 */

namespace gc {

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
			collector::singleton.insert_source(src, trav);
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
			std::lock_guard<std::mutex> l{collector::singleton.mtx};
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
			{
				auto &vec = collector::singleton.sources;
				auto tag = vec.cbegin();
				for (auto cur = begin; cur != end; cur++) {
					collector::singleton.erase_source(vec, tag, static_cast<isource const &>(*cur));
				}
			}
			if (trav) {
				auto &vec = collector::singleton.traversable;
				auto tag = vec.cbegin();
				for (auto cur = begin; cur != end; cur++) {
					collector::singleton.erase_source(vec, tag, static_cast<isource const &>(*cur));
				}
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
		 *	\details units that is approximately (vmem::page_size * config::max_pool).
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

	/**	\typedef source
	 *	\brief The type stored in sources.
	 */
	using source = isource *;

	/**	\typedef alloc_ptr
	 *	\brief The type for stored allocators.
	 */
	using alloc_ptr = std::unique_ptr<iallocator>;

	/**	\struct mroot
	 *	\brief The type to store roots to managed objects.
	 *	\see managed
	 */
	struct mroot
	{
		/**	\var obj
		 *	\brief A pointer to the root object.
		 */
		void *obj;

		/**	\var src
		 *	\brief The source of the root object.
		 */
		source src;
	};

	/**	\struct uroot
	 *	\brief The type to store roots to unmanaged objects.
	 *	\see unmanaged
	 */
	struct uroot
	{
		/**	\var obj
		 *	\brief A pointer to the root object.
		 */
		void *obj;

		/**	\var tcb
		 *	\brief The callback for traversing pointers in the object.
		 */
		traverse_cb tcb;

		/**	\var rcb
		 *	\brief The callback for updating pointers in the object.
		 */
		root_cb rcb;
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
	 *	\invariant Should be kept sorted by location in memory.
	 */
	std::vector<source> sources;

	/**	\var traversable
	 *	\brief The traversable sources.
	 *	\invariant Should be kept sorted by location in memory.
	 */
	std::vector<source> traversable;

	/**	\var allocators
	 *	\brief Tracks allocators for shrink request.
	 *	\invariant Should be kept sorted by location in memory.
	 */
	std::vector<alloc_ptr> allocators;

	/**	\var managed
	 *	\brief Tracks root objects with lifetimes managed by the collector.
	 *	\invariant Should be kept sorted by address of the object.
	 */
	std::vector<mroot> managed;

	/**	\var unmanaged
	 *	\brief Tracks root objects with lifetimes not managed by the collector.
	 *	\invariant Should be kept sorted by address of the object.
	 */
	std::vector<uroot> unmanaged;

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

	/**	\fn insert_helper(std::vector<T1> &vec, T2 const &find, T1 &&add, F &func, std::unique_lock<std::mutex> &l)
	 *	\param vec The vector to insert the root into.
	 *	\param find The object to search for the insert location.
	 *	\param add The object record to insert.
	 *	\param func A function to check find < element from vec.
	 *	\param l The the lock.
	 *	\pre The lock l must own the lock on mtx.
	 *	\post The lock l will own the lock on mtx.
	 */
	template<typename T1, typename T2, typename F>
	void insert_helper(
		std::vector<T1> &vec, T2 const &find, T1 &&add, F &func, std::unique_lock<std::mutex> &l);

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

	/**	\fn erase_source(std::vector<source> &vec, decltype(vec.cbegin()) &start, isource const &src) noexcept
	 *	\brief Stops tracking a source.
	 *	\param vec Either sources or traversable to remove the source from.
	 *	\param start The start of the search range. Updated to the location of the removed source.
	 *	\param src The source to remove.
	 */
	void erase_source(
		std::vector<source> &vec, decltype(vec.cbegin()) &start, isource const &src) noexcept;

	/**	\fn insert_alloc_impl(alloc_ptr &&alloc) noexcept
	 *	\brief Takes ownership of an allocator.
	 *	\param alloc The source to take ownership of.
	 *	\returns Returns a pointer to the inserted allocator.
	 */
	iallocator *insert_alloc_impl(alloc_ptr &&alloc) noexcept;

	/**	\fn erase_alloc_impl(iallocator const &alloc) noexcept
	 *	\brief Removes an allocator.
	 *	\param alloc The allocator to remove.
	 */
	void erase_alloc_impl(iallocator const &alloc) noexcept;

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

	/**	\fn collect() noexcept
	 *	\brief Causes a collection cycle to run as soon as possible.
	 */
	static void collect() noexcept { singleton.collect_impl(); }

	/**	\fn base_ptr(void *ptr) noexcept
	 *	\brief Gets the originally allocated address.
	 *	\param ptr The pointer to get the base address for.
	 *	\returns Returns the address originally allocated for the object ptr points to a
	 *	\returns location within. Or nullptr if not allocated by a registered source.
	 */
	static void *base_ptr(void *ptr) noexcept { return singleton.base_ptr_impl(ptr); }

	/**	\fn get_type(void *ptr) noexcept
	 *	\brief Gets the originally allocated address.
	 *	\param ptr The pointer to get the identity for.
	 *	\returns Returns the identity of an object or nullptr if not from a registered source.
	 */
	static identity const *get_type(void *ptr) noexcept { return singleton.get_type_impl(ptr); }

	/**	\fn register_root(void *obj, traverse_cb tcb, root_cb rcb)
	 *	\brief Registers a root object with the collector.
	 *	\param obj A pointer to the root object being registered.
	 *	\param tcb The callback for traversing pointers in the object.
	 *	\param rcb The callback for updating pointers in the object.
	 */
	static void register_root(void *obj, traverse_cb tcb, root_cb rcb)
	{
		return singleton.register_root_impl(obj, tcb, rcb);
	}

	/**	\fn unregister_root(void *obj) noexcept
	 *	\brief Unregisters a root object.
	 *	\param obj A pointer to the root object to unregister.
	 */
	static void unregister_root(void *obj) noexcept { singleton.unregister_root_impl(obj); }

	/**	\fn set_period(duration value) noexcept
	 *	\brief Sets the period for singleton.
	 *	\param value The new value for period.
	 */
	static void set_period(duration value) noexcept { singleton.set_period_impl(value); }

	/**	\fn insert_alloc(alloc_ptr &&alloc) noexcept
	 *	\brief Takes ownership of an allocator.
	 *	\param alloc The source to take ownership of.
	 *	\returns Returns a pointer to the inserted allocator.
	 */
	static iallocator *insert_alloc(alloc_ptr &&alloc) noexcept
	{
		return singleton.insert_alloc_impl(std::forward<alloc_ptr>(alloc));
	}

	/**	\fn erase_alloc(iallocator const &alloc) noexcept
	 *	\brief Removes an allocator.
	 *	\param alloc The allocator to remove.
	 */
	static void erase_alloc(iallocator const &alloc) noexcept { singleton.erase_alloc_impl(alloc); }

	/**	\fn find_source(void *ptr) noexcept
	 *	\brief Finds the source an address was allocated from.
	 *	\param ptr The pointer to get a base address for.
	 *	\returns Returns the source of ptr or nullptr if none.
	 */
	static isource *find_source(void *ptr) noexcept { return singleton.find_source_wrap(ptr); }
};

}

#endif
