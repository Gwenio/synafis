
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

/**	\file test/gc/pool.cpp
 *	\brief Defines test cases for gc::pool.
 */

#include "../../src/gc/pool.hpp"
#include "../master.hpp"

#include <stdexcept>
#include <windows.h>

namespace unit_test {

/**	\class tester<gc::pool>
 *	\brief Class with access to the interals of gc::pool to make tests.
 */
template<>
class tester<gc::pool> {
	/**	\fn tester()
	 *	\brief Deleted.
	 */
	tester() = delete;

	/**	\fn ~tester()
	 *	\brief Deleted.
	 */
	~tester() = delete;
public:
	/**	\typedef pool
	 *	\brief Shorthand for the class being tested.
	 */
	typedef gc::pool pool;

	/**	\typedef handle
	 *	\brief Shorthand for a pool::handle.
	 */
	typedef pool::handle handle;
private:
public:
};

}

namespace {

using t = unit_test::tester<gc::pool>;
using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

static unit_test::suite s{"pool", unit_test::gc_suite};

}
