
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

/**	\file test/unit_runner.cpp
 *	\brief The test runner for unit tests.
 */

#include "master.hpp"
#include "../src/gc.hpp"

#include <atomic>
#include <mutex>
#include <iostream>
#include <iomanip>
#include <stack>
#include <vector>
#include <string>

namespace {

using ::unit_test::status;

/**	\var current_status
 *	\brief The status of the current test case.
 */
static std::atomic<status> current_status;

/**	\fn indentl(std::ostream &out, std::size_t indent)
 *	\brief Outputs indentation for the start of a line.
 *	\param out The output stream to indent.
 *	\param indent The size of the indentation.
 *	\returns out
 */
std::ostream &indentl(std::ostream &out, std::size_t indent) {
	for (std::size_t x = 0; x < indent; x++) {
		out << "  ";
	}
	return out;
}

/**	\class collector_impl
 *	\brief Implementation of unit_test::collector.
 *	\todo For now just outputs a summary to the console.
 *	\todo In the future it would be good to output to files and
 *	\todo have the option for JSON output.
 */
class collector_impl : public ::unit_test::collector {
public:
	/**	\class results
	 *	\brief Stores test cases that have not completed.
	 */
	class results {
	private:
		/**	\typedef msg_tuple
		 *	\brief Shorthand for container of messages.
		 */
		typedef std::tuple<std::string, int, std::string_view> msg_tuple;

		/**	\var name
		 *	\brief The name of the test case.
		 */
		std::string_view name;

		/**	\var expect
		 *	\brief The expected result.
		 */
		status expect;

		/**	\var messages
		 *	\brief List of messages.
		 */
		std::vector<msg_tuple> messages;

		/**	\fn print_status(status s, std::ostream &out)
		 *	\brief Prints a text output for a status.
		 *	\param s The status to print.
		 *	\param out The stream to output to.
		 *	\Returns A reference to 'out'.
		 */
		static std::ostream &print_status(status s, std::ostream &out) {
			switch(s) {
			case unit_test::pass:
				out << "pass";
				break;
			case unit_test::fail:
				out << "fail";
				break;
			case unit_test::skip:
				out << "skip";
				break;
			default:
				out << "unknown";
				break;
			}
			return out;
		}

		/**	\fn print_message(std::ostream &out, msg_tuple const& msg, std::size_t indent)
		 *	\brief Prints a message.
		 *	\param out The output sink.
		 *	\param msg The message to output.
		 *	\param indent The base indentation for the output.
		 */
		static void print_message(std::ostream &out, msg_tuple const& msg, std::size_t indent) {
			indentl(out, indent) << std::get<2>(msg) << " @ " << std::get<1>(msg) << std::endl;
			indentl(out, indent) << std::get<0>(msg) << std::endl;
		}
	public:
		/**	\fn results()
		 *	\brief Deleted.
		 */
		results() = delete;

		/**	\fn results(std::string_view n, status ex)
		 *	\brief Constructs a new results object.
		 *	\param n The name of the test case.
		 *	\param ex The expected result.
		 */
		results(std::string_view n, status ex) : name(n), expect(ex), messages() {}

		/**	\fn ~results()
		 *	\brief Default.
		 */
		~results() = default;

		/**	\fn append(std::string_view msg, int lineno, std::string_view file)
		 *	\brief Appends a new message.
		 *	\param msg A message from the current test case.
		 *	\param lineno The line number of the message.
		 *	\param file The file the message is from.
		 */
		void append(std::string_view msg, int lineno, std::string_view file) {
			messages.emplace_back(msg, lineno, file);
		}

		/**	\fn print(std::ostream &out, status end, std::size_t indent) const
		 *	\brief Prints a basic output of the results.
		 *	\param out The output sink.
		 *	\param end The final status of the test case.
		 *	\param indent The base indentation for the output.
		 */
		void print(std::ostream &out, status end, std::size_t indent) const {
			print_status(end, indentl(out, indent) << name << " :\t");
			if (expect != end) {
				print_status(expect, out << "(expected ") << ")";
			}
			out << std::endl;
			indent++;
			for (auto m : messages) {
				print_message(out, m, indent);
			}
			out << std::endl;
		}
	};
private:
	/**	\var msg_lock
	 *	\brief A mutex for output synchronisation.
	 */
	std::mutex msg_lock;

	/**	\var indent
	 *	\brief The amount of indent.
	 */
	std::size_t indent;

	/**	\var cases
	 *	\brief A stack of unfinished test cases.
	 */
	std::stack<results> cases;
public:
	/**	\fn collector_impl()
	 *	\brief Default.
	 */
	collector_impl() = default;

	/**	\fn ~collector_impl() noexcept
	 *	\brief Default.
	 */
	virtual ~collector_impl() noexcept = default;

	/**	\fn next(std::string_view name) override
	 *	\brief Move to the next sibling of the current test suite.
	 *	\param name The name of the test suite we are switching to.
	 */
	virtual void next(std::string_view name) override {
		std::lock_guard<std::mutex> l{msg_lock};
		indentl(std::cout, indent) << name << std::endl;
	}

	/**	\fn begin(std::string_view name, status expect) noexcept override
	 *	\brief Indicates a test case will begin running.
	 *	\param name The name of the test case that is about to begin.
	 *	\param expect The expected result of the test case.
	 */
	virtual void begin(std::string_view name, status expect) noexcept override {
		std::lock_guard<std::mutex> l{msg_lock};
		cases.emplace(name, expect);
	}

	/**	\fn end(status result) noexcept override
	 *	\brief Indicates the test case has ended.
	 *	\param result The result of the test case.
	 */
	virtual void end(status result) noexcept override {
		std::lock_guard<std::mutex> l{msg_lock};
		cases.top().print(std::cout, result, indent);
		cases.pop();
	}

	/**	\fn message(std::string_view msg, int lineno, std::string_view file) noexcept override
	 *	\brief The method for test cases to output messages.
	 *	\param msg A message from the current test case.
	 *	\param lineno The line number of the message.
	 *	\param file The file the message is from.
	 */
	virtual void message(std::string_view msg, int lineno, std::string_view file) noexcept override {
		std::lock_guard<std::mutex> l{msg_lock};
		cases.top().append(msg, lineno, file);
	}

	/**	\fn up() override
	 *	\brief Move to the first child of the current test suite.
	 */
	virtual void up() override {
		std::lock_guard<std::mutex> l{msg_lock};
		indent++;
	}

	/**	\fn down() override
	 *	\brief All children of a test suite have completed.
	 */
	virtual void down() override {
		std::lock_guard<std::mutex> l{msg_lock};
		indent--;
	}

	/**	\fn init(std::string_view start)
	 *	\brief Sets up the collector.
	 *	\param start The name of the primary test suite.
	 */
	void init(std::string_view start) {
		indent = 0;
	}
};

/**	\var master
 *	\brief The primary test suite all other will be children of.
 */
extern unit_test::suite master{"master"};

/**	\var output
 *	\brief The collector of the test runner.
 */
static collector_impl output{};

}

namespace unit_test {

unit_test::suite gc_suite{"gc", master};

void suite::run(collector &out, suite &root) {
	suite *current{std::addressof(root)};
	//do {
		out.next(current->name);
		{
			suite *child{root.children};
			out.up();
			while (child) {
				run(out, *child);
				child = child->next;
			}
			out.down();
		}
		out.up();
		case_type::each(root.cases, [&out](case_type &c) -> void {
			c(out);
		});
		out.down();
		/*current = current->next;
	} while(current);*/
}

case_type::context::context() noexcept : saved(current_status.load()) {
	current_status.store(pass);
}

case_type::context::~context() noexcept {
	current_status.store(saved);
}

void case_type::context::failed() noexcept {
	current_status.store(fail);
}

void case_type::context::skipping() noexcept {
	status expected{pass};
	current_status.compare_exchange_strong(expected, skip);
}

status case_type::context::get() noexcept {
	return current_status.load();
}

void fail_msg(std::string_view msg, int lineno, std::string_view file) noexcept {
	case_type::context::failed();
	output.message(msg, lineno, file);
}

}

/**	\fn main()
 *	\brief The main function of the unit test runner.
 *	\todo Process command line arguments to control the output.
 */
int main() {
	std::cout << "main()" << std::endl;
	gc::initialize();
	output.init("master");
	std::cout << "Beginning testing..." << std::endl;
	unit_test::suite::run(output, master);
	std::cout << "Testing complete." << std::endl;
	return 0;
}
