
/*
ISC License (ISC)

Copyright 2019 Adam Armstrong

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

/**	\file test/gc/pool/suites.hpp
 *	\brief Declares the test suites for the sub-components of gc::pool.
 */

#ifndef SYNAFIS_TEST_GC_POOL_SUITES_HPP
#define SYNAFIS_TEST_GC_POOL_SUITES_HPP
#pragma once

#include "../gc.hpp"

namespace gc_test {

// Declare the pool test suites.

/**	\var blueprint_suite
 *	\brief The test suite for ::gc::blueprint.
 */
extern suite blueprint_suite;

/**	\var gray_list_suite
 *	\brief The test suite for ::gc::gray_list.
 */
extern suite gray_list_suite;

/**	\var free_list_suite
 *	\brief The test suite for ::gc::free_list.
 */
extern suite free_list_suite;

/**	\var arena_suite
 *	\brief The test suite for ::gc::arena.
 */
extern suite arena_suite;

}

#endif
