
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

#ifndef SYNAFIS_UNIT_TEST_CASE_TYPE_HPP
#define SYNAFIS_UNIT_TEST_CASE_TYPE_HPP
#pragma once

#ifndef SYNAFIS_UNIT_TEST_HPP
#include "../unit_test.hpp"
#endif

#ifndef SYNAFIS_UNIT_TEST_STATUS_HPP
#include "status.hpp"
#endif

#ifndef SYNAFIS_UNIT_TEST_COLLECTOR_HPP
#include "collector.hpp"
#endif

#ifndef SYNAFIS_UNIT_TEST_SUITE_HPP
#include "suite.hpp"
#endif

#include <string_view>
#include <optional>

/**	\file src/unit_test/case_type.hpp
 *	\brief Defines type for test cases.
 */
namespace unit_test {

/**	\class case_type
 *	\brief Represents a test case.
 */
class case_type {
public:
	/**	\class context
	 *	\brief Saves the current test status when running another.
	 */
	class context {
	private:
		/**	\var saved
		 *	\brief The saved status.
		 */
		status saved;
	public:
		/**	\fn context() noexcept
		 *	\brief Puts the previous status in saved and sets status to pass.
		 */
		context() noexcept;

		/**	\fn ~context() noexcept
		 *	\brief Sets saved as the current status.
		 *	\warning Any additional threads spawned by the test must exit before this.
		 */
		~context() noexcept;

		/**	\fn get() noexcept
		 *	\brief Gets the current status.
		 *	\returns Returns the current test status.
		 */
		static status get() noexcept;

		/**	\fn skipping() noexcept
		 *	\brief Sets the current status to skip if it was pass beforehand.
		 *	\details Call to indicate a test is skipped based on the results of other tests.
		 *	\details
		 *	\details If the test status is fail then it will not be changed.
		 *	\warning Skipping should be called before spawning any threads in a test.
		 */
		static void skipping() noexcept;

		/**	\fn failed() noexcept
		 *	\brief Sets the current test status to fail.
		 *	\note In most cases fail_msg should be called to signal failure.
		 */
		static void failed() noexcept;
	};

	/**	\var test_cb
	 *	\brief The function pointer type for test functions.
	 */
	typedef void (*test_cb)(collector &);

	/**	\var name
	 *	\brief The name of the test case.
	 */
	std::string_view const name;

	/**	\var expected
	 *	\brief The expected result.
	 *	\note If the value is skip, the test will be skipped.
	 */
	status const expected;
private:
	/**	\var next
	 *	\brief The next test case.
	 */
	case_type *next;

	/**	\var result
	 *	\brief The result of the test.
	 *	\note If 'expected' is skip, then result will be set to skip without running the test.
	 */
	std::optional<status> result;

	/**	\var test
	 *	\brief The function to run for the test.
	 *	\note If test is nullptr, then result will be skip.
	 */
	test_cb const test;

	/**	\fn case_type(std::string_view n, status ex, test_cb cb) noexcept
	 *	\brief Preforms the constexpr portion of initializing a test case.
	 *	\param n The name of the test case.
	 *	\param ex The expected result.
	 *	\param cb The function to preform the test.
	 */
	constexpr case_type(std::string_view n, status ex, test_cb cb) noexcept :
		name(n), expected(ex), next(nullptr), result(std::nullopt), test(cb) {}
public:
	/**	\fn case_type(std::string_view n, suite &parent, status ex, test_cb cb) noexcept
	 *	\brief Preforms the constexpr portion of initializing a test case.
	 *	\param n The name of the test case.
	 *	\param parent The test suite the test case belongs to.
	 *	\param ex The expected result.
	 *	\param cb The function to preform the test.
	 *	\pre 'n' should be initialized with a string literal so that it is not deallocated.
	 *	\warning It is assumed that static variable contructors are run on a single thread and
	 *	\warning that the constexpr initialization is preformed for all suites before non-constexpr
	 *	\warning constructors.
	 *	\note Test cases will be run in the opposite order they are added to a suite. (LIFO)
	 */
	case_type(std::string_view n, suite &parent, status ex, test_cb cb) noexcept :
			case_type(n, ex, cb) {
		next = std::exchange(parent.cases, this);
		if (expected == skip || !test) {
			result = skip;
		}
	}

	/**	\fn ~case_type() noexcept
	 *	\brief Default.
	 */
	~case_type() noexcept = default;

	/**	\fn operator()(collector &out) noexcept
	 *	\brief Gets the result of the test case, running it if needed.
	 *	\param out The collector for test output.
	 *	\returns Returns the result of the test.
	 *	\details Creates a new context.
	 *	\details
	 *	\details Test cases may call this operator on other cases
	 *	\details before starting their own testing to check for
	 *	\details conditions where they should be skip.
	 *	\details However, to give the collector correct singals,
	 *	\details only tests from the same suite or from children
	 *	\details of that suite should have their results checked.
	 */
	status operator()(collector &out) noexcept {
		if (!result) {
			context ctx{};
			out.begin(name, expected);
			try {
				(*test)(out);
			} catch(...) {
				fail_msg("The test case exited with an unexpected exception.", __LINE__, __FILE__);
			}
			status temp{context::get()};
			out.end(temp);
			result = temp;
			return temp;
		} else {
			return *result;
		}
	}

	/**	\fn each(case_type *current, Func &func)
	 *	\brief Iterates through sibling test cases calling func on each one.
	 *	\tparam A callable type accepting a reference to a case_type.
	 *	\param current The current case.
	 *	\param func The function to pass a reference to each case to.
	 *	\details Used by suite::run to iterate the test cases of a suite.
	 */
	template<typename Func>
	static void each(case_type *current, Func &func) {
		while(current) {
			func(*current);
			current = current->next;
		}
	}
};

}

#endif
