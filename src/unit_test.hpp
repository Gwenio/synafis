
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

#ifndef SYNAFIS_UNIT_TEST_HPP
#define SYNAFIS_UNIT_TEST_HPP

/**	\file unit_test.hpp
 *	\brief Defines utilities for constructing unit tests.
 */

/**	\namespace unit_test
 *	\brief Contains utilities for constructing unit tests.
 */
namespace unit_test {

/**	\class tester
 *	\brief Class to be a friend of others so they can be examined in tests.
 *	\tparam T The type for testing.
 */
template<typename T>
class tester;

}

#if defined(SYNAFIS_UNIT_TEST)
#warning Unit Test macros are not yet implemented.
#define SYNAFIS_ASSERT(x)
#elif !defined(NDEBUG)
// Use generic assert for debug without unit testing.
#include <cassert>
#define SYNAFIS_ASSERT(x) assert(x)
#else
#define SYNAFIS_ASSERT(x)
#endif

#endif
