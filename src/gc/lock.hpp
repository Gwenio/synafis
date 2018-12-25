
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

#ifndef SYNAFIS_GC_LOCK_HPP
#define SYNAFIS_GC_LOCK_HPP
#pragma once

/**	\file src/gc/lock.hpp
 *	\brief Defines tools for preventing garbage collection during mutation.
 */

namespace gc {

/**	\class scoped_lock
 *	\brief Class to ensure the garbage collector does not run.
 *	\note The constructor and destructor are to be implemented inside the collector.
 */
class scoped_lock {
public:
	/**	\fn scoped_lock()
	 *	\brief Acquires the lock.
	 */
	scoped_lock();
	/**	\fn ~scoped_lock()
	 *	\brief Releases the lock.
	 */
	~scoped_lock();
};

}

#endif
