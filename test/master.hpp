
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

#ifndef SYNAFIS_TEST_MASTER_HPP
#define SYNAFIS_TEST_MASTER_HPP
#pragma once

/**	\file test/master.hpp
 *	\brief The master include file for tests.
 *	\details Include important test related headers.
 *	\details Declares the main test suites.
 */

// Include unit test infostructure.

#include "../src/unit_test.hpp"
#include "../src/unit_test/status.hpp"
#include "../src/unit_test/collector.hpp"
#include "../src/unit_test/suite.hpp"
#include "../src/unit_test/case_type.hpp"

namespace unit_test {

// Declare the main test suites.

/**	\var gc_suite
 *	\brief The main test suite for the garbage collector.
 */
static suite gc_suite;

}

#endif
