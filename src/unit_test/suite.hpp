
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

#ifndef SYNAFIS_UNIT_TEST_SUITE_HPP
#define SYNAFIS_UNIT_TEST_SUITE_HPP
#pragma once

#include <string_view>

/**	\file src/unit_test/suite.hpp
 *	\brief Defines the suite class for grouping tests.
 */
namespace unit_test {

class collector;
class case_type;

/**	\class suite
 *	\brief Used to manage test cases.
 */
class suite {
	friend case_type;
private:
	/**	\var name
	 *	\brief The name of the suite.
	 */
	std::string_view const name;

	/**	\var next
	 *	\brief A pointer to the next sibling or nullptr if there is not one.
	 */
	suite *next;

	/**	\var children
	 *	\brief A pointer to the first child or nullptr if there is not one.
	 */
	suite *children;

	/**	\var cases
	 *	\brief A pointer to the first test case or nullptr if there is not one.
	 */
	case_type *cases;
public:
	/**	\fn suite(std::string_view n) noexcept
	 *	\brief Initializes a suite in a constexpr manner.
	 *	\param n The name of the suite.
	 *	\pre 'n' should be initialized with a string literal so that it is not deallocated.
	 *	\details Has no parent suite.
	 */
	constexpr suite(std::string_view n) noexcept : name(n),
		next(nullptr), children(nullptr), cases(nullptr) {}

	/**	\fn suite(std::string_view n, suite &parent) noexcept
	 *	\brief Initializes a child suite.
	 *	\param n The name of the suite.
	 *	\pre 'n' should be initialized with a string literal so that it is not deallocated.
	 *	\warning It is assumed that static variable contructors are run on a single thread and
	 *	\warning that the constexpr initialization is preformed for all suites before the part
	 *	\warning that is not constexpr.
	 */
	suite(std::string_view n, suite &parent) noexcept : suite(n) {
		next = std::exchange(parent.children, this);
	}

	/**	\fn ~suite() noexcept
	 *	\brief Default.
	 */
	~suite() noexcept = default;

	/**	\fn run(collector &out, suite &root)
	 *	\brief Run the test cases of a suite, its children, and its siblings.
	 *	\param out The collector of test results.
	 *	\param root The initial suite.
	 *	\note Child suites are to be run before the test cases of the suite.
	 */
	static void run(collector &out, suite &root);
};

}

#endif
