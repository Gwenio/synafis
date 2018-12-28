
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
#pragma once

/**	\file src/unit_test.hpp
 *	\brief Defines utilities for constructing unit tests.
 */

/**	\dir src/unit_test
 *	\brief Contains the headers defining testing infostructure.
 */

#include <string_view>

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

/**	\fn fail_msg(std::string_view msg, int lineno, std::string_view file) noexcept
 *	\brief Sets the current test status to fail.
 *	\param msg A message about why the test failed.
 *	\param lineno The line number the failure occurred at.
 *	\param file The file the failure occurred in.
 *	\details Defined in the test runner source.
 *	\note Thread safe.
 */
void fail_msg(std::string_view msg, int lineno, std::string_view file) noexcept;

}

/**	\def SYNAFIS_ASSERT(x)
 *	\brief Determine if if the expression 'x' is true.
 *	\param x The expression to evaluate.
 *	\pre 'x' should not have side affects.
 *	\details In unit testing will case the current case to fail if evaluates false.
 *	\details
 *	\details For debugging will use assert(x) from \<cassert\>.
 *	\details
 *	\details Otherwise will be omitted so as to not impact release proformace.
 */

/**	\def SYNAFIS_FAILURE(msg)
 *	\brief Generates a failed assertion.
 *	\param msg A string literal containing a message explaining the failure.
 *	\details In unit testing will case the current case to fail.
 *	\details
 *	\details For debugging will use assert(false && msg) from \<cassert\>.
 *	\details
 *	\details Otherwise will be omitted so as to not impact release proformace.
 */
#if defined(SYNAFIS_UNIT_TEST)

#define SYNAFIS_ASSERT(x) \
	if (!(x)) { ::unit_test::fail_msg(#x, __LINE__, __FILE__); }

#define SYNAFIS_FAILURE(msg)                            \
	{                                                   \
		::unit_test::fail_msg(msg, __LINE__, __FILE__); \
	}

#elif !defined(NDEBUG)
// Use generic assert for debug without unit testing.
#include <cassert>
#define SYNAFIS_ASSERT(x) assert(x)
#define SYNAFIS_FAILURE(msg) assert(false && msg)
#else
#define SYNAFIS_ASSERT(x)
#define SYNAFIS_FAILURE(msg)
#endif

#endif
