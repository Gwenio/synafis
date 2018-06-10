
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

#ifndef SYNAFIS_CONFIG_GC_HPP
#define SYNAFIS_CONFIG_GC_HPP
#pragma once

#ifndef SYNAFIS_CONFIG_GENERAL_HPP
#include "general.hpp"
#endif

/**	\file config/gc.hpp
 *	\brief Configures the garbage collector.
 */

// Set default configurations for those not specified.

#ifndef SYNAFIS_CONFIG_GUARD_PAGES
#define SYNAFIS_CONFIG_GUARD_PAGES (config::debug ? 1 : 0)
#endif

namespace config {

/**	\var guard_pages
 *	\brief Controls the presence and number of guard pages for blocks of virtual memory.
 *	\details Set to the preprocessor definition SYNAFIS_CONFIG_GUARD_PAGE.
 *	\details A value of zero disables the use of guard pages.
 *	\details
 *	\details Guard pages are page(s) of inaccessible memory before and
 *	\details after a block of allocated virtual memory.
 *	\details This detects some memory issues as an error will occur
 *	\details if the program touches a guard page.
 *	\details Due to how virtual memory is used in the collector, the detection
 *	\details provided by this alone is limited.
 *	\note As virtual memory is system specific, guard pages are only possible
 *	\note if system support is present.
 *	\note However, it would be suprising if a modern fully featured system lacked support.
 */
inline constexpr std::size_t const guard_pages = SYNAFIS_CONFIG_GUARD_PAGES;

}

#endif
