
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

#ifndef SYNAFIS_UNIT_TEST_COLLECTOR_HPP
#define SYNAFIS_UNIT_TEST_COLLECTOR_HPP
#pragma once

#ifndef SYNAFIS_UNIT_TEST_STATUS_HPP
#include "status.hpp"
#endif

#include <string_view>

/**	\file src/unit_test/collector.hpp
 *	\brief Defines the interface for collecting test results.
 */

namespace unit_test {

/**	\class collector
 *	\brief The interface for collecting test results.
 */
class collector
{
protected:
	/**	\fn collector() noexcept
	 *	\brief Default.
	 */
	collector() noexcept = default;

public:
	/**	\fn ~collector() noexcept
	 *	\brief Default.
	 */
	virtual ~collector() noexcept = default;

	/**	\fn next(std::string_view name)
	 *	\brief Move to the next sibling of the current test suite.
	 *	\param name The name of the test suite we are switching to.
	 */
	virtual void next(std::string_view name) = 0;

	/**	\fn begin(std::string_view name, status expect) noexcept
	 *	\brief Indicates a test case will begin running.
	 *	\param name The name of the test case that is about to begin.
	 *	\param expect The expected result of the test case.
	 */
	virtual void begin(std::string_view name, status expect) noexcept = 0;

	/**	\fn end(status result) noexcept
	 *	\brief Indicates the test case has ended.
	 *	\param result The result of the test case.
	 */
	virtual void end(status result) noexcept = 0;

	/**	\fn message(std::string_view msg, int lineno, std::string_view file) noexcept
	 *	\brief The method for test cases to output messages.
	 *	\param msg A message from the current test case.
	 *	\param lineno The line number of the message.
	 *	\param file The file the message is from.
	 */
	virtual void message(std::string_view msg, int lineno, std::string_view file) noexcept = 0;

	/**	\fn up()
	 *	\brief Move to the first child of the current test suite.
	 */
	virtual void up() = 0;

	/**	\fn down()
	 *	\brief All children of a test suite have completed.
	 */
	virtual void down() = 0;
};

}

#endif
