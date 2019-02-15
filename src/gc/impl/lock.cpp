
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

/**	/file src/gc/impl/lock.cpp
 *	/brief Implements gc::basic_mutex.
 */

#include "../lock.hpp"
#include "core.hpp"

#if defined(SYNAFIS_UNIT_TEST) || !defined(NDEBUG)
#include <limits>
#endif

namespace gc {

/**	\var lock_count
 *	\brief Count how many times a thread has acquired the collector's lock.
 */
thread_local std::size_t lock_count{0};

void basic_mutex::lock()
{
	SYNAFIS_ASSERT(lock_count < std::numeric_limits<std::size_t>::max());
	if (lock_count++ == 0) { collector::lock(); }
}

void basic_mutex::unlock()
{
	SYNAFIS_ASSERT(0 < lock_count);
	if (--lock_count == 0) { collector::unlock(); }
}

bool basic_mutex::locked() noexcept { return 0 < lock_count; }

}
