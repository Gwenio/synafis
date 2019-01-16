
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

#include "../config/gc.hpp"
#include "../unit_test.hpp"

#ifndef SYNAFIS_GC_LOCK_HPP
#define SYNAFIS_GC_LOCK_HPP
#pragma once

#include <mutex>
#include <thread>

/**	\file src/gc/lock.hpp
 *	\brief Defines tools for preventing garbage collection during mutation.
 */

namespace gc {

/**	\class basic_mutex
 *	\brief Class to ensure the garbage collector does not run.
 *	\note The constructor and destructor are to be implemented inside the collector.
 */
class basic_mutex
{
	/**	\fn basic_mutex(basic_mutex const &)
	 *	\brief Deleted.
	 */
	basic_mutex(basic_mutex const &) = delete;

	/**	\fn basic_mutex(basic_mutex &&)
	 *	\brief Deleted.
	 */
	basic_mutex(basic_mutex &&) = delete;

	/**	\fn operator=(basic_mutex const &)
	 *	\brief Deleted.
	 *	\returns *this
	 */
	basic_mutex &operator=(basic_mutex const &) = delete;

	/**	\fn operator=(basic_mutex &&)
	 *	\brief Deleted.
	 *	\returns *this
	 */
	basic_mutex &operator=(basic_mutex &&) = delete;

public:
	/**	\fn basic_mutex() noexcept
	 *	\brief Default constructor.
	 */
	constexpr basic_mutex() noexcept = default;

	/**	\fn ~basic_mutex() noexcept
	 *	\brief Default destructor.
	 */
	~basic_mutex() noexcept = default;

	/**	\fn lock()
	 *	\brief Acquires the collector lock.
	 *	\details The lock may be acquired multiple times, but must be released an equal number of times.
	 *	\details All instances of mutex have the same lock count for a given thread.
	 */
	void lock();

	/**	\fn unlock()
	 *	\brief Releases the collector lock.
	 *	\details Must be called an equal number of times as lock() before actually releasing the lock.
	 *	\details All instances of mutex have the same lock count for a given thread.
	 */
	void unlock();

	/**	\fn try_lock()
	 *	\brief Simply calls lock().
	 *	\returns true
	 */
	bool try_lock()
	{
		lock();
		return true;
	}

	/**	\fn locked() noexcept
	 *	\brief Checks if the thread has the collector lock.
	 *	\returns Returns true if the collector has been locked by the calling thread.
	 */
	static bool locked() noexcept;
};

/**	\class debug_mutex
 *	\brief Class to ensure the garbage collector does not run.
 *	\note The constructor and destructor are to be implemented inside the collector.
 */
class debug_mutex : public basic_mutex
{
private:
	/**	\var thread_id
	 *	\brief The id of the thread that created the debug_mutex.
	 *	\details The methods lock() and unlock() must only be called from the thread with id.
	 */
	std::thread::id thread_id;

public:
	/**	\fn debug_mutex() noexcept
	 *	\brief Constructs with thread_id == std::this_thread::get_id().
	 */
	debug_mutex() noexcept : basic_mutex(), thread_id(std::this_thread::get_id()) {}

	/**	\fn ~debug_mutex() noexcept
	 *	\brief Default destructor.
	 */
	~debug_mutex() noexcept = default;

	/**	\fn lock()
	 *	\brief Wrapper for basic_mutex::lock().
	 *	\details Asserts that the calling thread is the one that constructed the mutex.
	 */
	void lock()
	{
		SYNAFIS_ASSERT(std::this_thread::get_id() == thread_id);
		basic_mutex::lock();
	}

	/**	\fn unlock()
	 *	\brief Wrapper for basic_mutex::unlock().
	 *	\details Asserts that the calling thread is the one that constructed the mutex.
	 */
	void unlock()
	{
		SYNAFIS_ASSERT(std::this_thread::get_id() == thread_id);
		basic_mutex::unlock();
	}

	/**	\fn try_lock()
	 *	\brief Simply calls lock().
	 *	\returns true
	 */
	bool try_lock()
	{
		lock();
		return true;
	}

	/**	\fn locked() noexcept
	 *	\brief Checks if the thread has the collector lock.
	 *	\returns Returns true if the collector has been locked by the calling thread.
	 */
	static bool locked() noexcept { return basic_mutex::locked(); }
};

/**	\typedef mutex
 *	\brief The type of mutex to use for the collector lock.
 *	\details Selects based on config::gc_debug_mutex.
 *	\see config::gc_debug_mutex
 *	\see debug_mutex
 *	\see basic_mutex
 */
using mutex = typename std::conditional_t<config::gc_debug_mutex, debug_mutex, basic_mutex>;

/**	\typedef lock_guard
 *	\brief Alias for std::lock_guard<mutex>.
 */
using lock_guard = std::lock_guard<mutex>;

/**	\typedef scoped_lock
 *	\brief Alias for std::scoped_lock<mutex>.
 *	\warning Ownership of the lock must not be transferred to another thread.
 */
using scoped_lock = std::scoped_lock<mutex>;

/**	\typedef unique_lock
 *	\brief Alias for std::unique_lock<mutex>.
 *	\warning Ownership of the lock must not be transferred to another thread.
 */
using unique_lock = std::unique_lock<mutex>;

}

#endif
