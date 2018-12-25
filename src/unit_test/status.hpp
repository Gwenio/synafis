
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

#if !defined(SYNAFIS_UNIT_TEST_HPP)
#include "../unit_test.hpp"
#elif !defined(SYNAFIS_UNIT_TEST_STATUS_HPP)
#define SYNAFIS_UNIT_TEST_STATUS_HPP
#pragma once

/**	\file src/unit_test/status.hpp
 *	\brief Defines the result values of test cases.
 */

namespace unit_test {

/**	\enum status
 *	\brief The result values of test cases.
 *	\note Also used to indicate the expected result.
 */
enum status {
	/**	\var pass
	 *	\brief The test passed.
	 */
	pass,

	/**	\var fail
	 *	\brief The test failed.
	 */
	fail,

	/**	\var skip
	 *	\brief The test was skipped.
	 */
	skip
};

}

#endif
