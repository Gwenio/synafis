
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

#include "general.hpp"

#ifndef SYNAFIS_CONFIG_GC_HPP
#define SYNAFIS_CONFIG_GC_HPP
#pragma once

#include <cstddef>

/**	\file src/config/gc.hpp
 *	\brief Configures the garbage collector.
 */

// Set default configurations for those not specified.

/**	\def SYNAFIS_CONFIG_GUARD_PAGES
 *	\brief The value for config::guard_pages.
 *	\note Define this value in the compiler commandline.
 *	\see config::guard_pages
 */
#ifndef SYNAFIS_CONFIG_GUARD_PAGES
#define SYNAFIS_CONFIG_GUARD_PAGES (config::debug)
#endif

/**	\def SYNAFIS_CONFIG_MIN_POOL
 *	\brief The value for config::min_pool.
 *	\note Define this value in the compiler commandline.
 *	\see config::min_pool
 */
#ifndef SYNAFIS_CONFIG_MIN_POOL
#define SYNAFIS_CONFIG_MIN_POOL (sizeof(std::size_t) * 8)
#endif

/**	\def SYNAFIS_CONFIG_MAX_POOL
 *	\brief The value for config::max_pool.
 *	\note Define this value in the compiler commandline.
 *	\see config::max_pool
 */
#ifndef SYNAFIS_CONFIG_MAX_POOL
#define SYNAFIS_CONFIG_MAX_POOL (sizeof(std::size_t) * 8)
#endif

/**	\def SYNAFIS_CONFIG_GC_PERIOD
 *	\brief The value for config::gc_period.
 *	\note Define this value in the compiler commandline.
 *	\see config::gc_period
 */
#ifndef SYNAFIS_CONFIG_GC_PERIOD
#define SYNAFIS_CONFIG_GC_PERIOD 1000
#endif

/**	\def SYNAFIS_CONFIG_GC_DEBUG_MUTEX
 *	\brief The value for config::gc_debug_mutex.
 *	\note Define this value in the compiler commandline.
 *	\see config::gc_debug_mutex
 */
#ifndef SYNAFIS_CONFIG_GC_DEBUG_MUTEX
#define SYNAFIS_CONFIG_GC_DEBUG_MUTEX config::debug
#endif

/**	\def SYNAFIS_CONFIG_INIT_TRACKING
 *	\brief The value for config::init_tracking.
 *	\note Define this value in the compiler commandline.
 *	\see config::gc_debug_mutex
 */
#ifndef SYNAFIS_CONFIG_INIT_TRACKING
#define SYNAFIS_CONFIG_INIT_TRACKING config::debug
#endif

namespace config {

/**	\var guard_pages
 *	\brief Controls the presence and number of guard pages for blocks of virtual memory.
 *	\details Set to the preprocessor definition SYNAFIS_CONFIG_GUARD_PAGE.
 *	\details
 *	\details Guard pages are page(s) of inaccessible memory separating
 *	\details accessible regions.
 *	\details
 *	\details This detects some memory issues as an error will occur
 *	\details if the program touches a guard page.
 *	\details Due to how virtual memory is used in the collector, the detection
 *	\details provided by this alone is limited.
 */
inline constexpr bool const guard_pages = SYNAFIS_CONFIG_GUARD_PAGES;

/**	\var min_pool
 *	\brief The minimum number of objects a pool can hold.
 *	\details Set to the preprocessor definition SYNAFIS_CONFIG_MIN_POOL.
 */
inline constexpr std::size_t const min_pool = SYNAFIS_CONFIG_MIN_POOL;

/**	\var max_pool
 *	\brief The maximum number of pages to use for objects in a pool.
 *	\details Set to the preprocessor definition SYNAFIS_CONFIG_MAX_PAGE.
 *	\details
 *	\details The variable min_pool takes priority if min_pool objects
 *	\details need more space than max_pool pages.
 */
inline constexpr std::size_t const max_pool = SYNAFIS_CONFIG_MAX_POOL;

/**	\var gc_period
 *	\brief The default time to wait between unforced GC cycles.
 *	\details The value is the number of miliseconds between cycles.
 *	\details If zero, then collection cycles will only run when forced.
 *	\see gc::set_period()
 */
inline constexpr std::size_t const gc_period = SYNAFIS_CONFIG_GC_PERIOD;

/**	\var gc_debug_mutex
 *	\brief When true gc::debug_mutex will be used; otherwise, gc::basic_mutex will be used.
 *	\see gc::mutex
 */
inline constexpr bool const gc_debug_mutex = SYNAFIS_CONFIG_GC_DEBUG_MUTEX;

/**	\var init_tracking
 *	\brief When true allocators will always track if allocated memory is initialized.
 */
inline constexpr bool const init_tracking = SYNAFIS_CONFIG_INIT_TRACKING;
// TODO At this time config::init_tracking does nothing, as initialization is always tracked.

}

//	Remove preprocessor definitions that are no longer needed.
#undef SYNAFIS_CONFIG_GUARD_PAGES
#undef SYNAFIS_CONFIG_MIN_POOL
#undef SYNAFIS_CONFIG_MAX_POOL
#undef SYNAFIS_CONFIG_GC_PERIOD
#undef SYNAFIS_CONFIG_GC_DEBUG_MUTEX
#undef SYNAFIS_CONFIG_INIT_TRACKING

#endif
