
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

/**	\file test/gc/pool/blueprint.hpp
 *	\brief Declares test cases for gc::blueprint.
 */

#include "../../master.hpp"

namespace gc {

class blueprint;

}

namespace unit_test {

/**	\class tester<gc::blueprint>
 *	\brief Class with access to the internals of gc::blueprint to make tests.
 */
template<>
class tester<gc::blueprint>
{
	/**	\fn tester()
	 *	\brief Deleted.
	 */
	tester() = delete;

	/**	\fn ~tester()
	 *	\brief Deleted.
	 */
	~tester() = delete;

public:
	/**	\typedef blueprint
	 *	\brief Shorthand for the class being tested.
	 */
	using blueprint = typename gc::blueprint;

	/**	\fn creation(collector &)
	 *	\brief Tests the constructors.
	 */
	static void creation(collector &);

	/**	\fn prepare_region(collector &)
	 *	\brief Tests blueprint::prepare_region().
	 */
	static void prepare_region(collector &);

	/**	\fn prepare_gray(collector &)
	 *	\brief Tests blueprint::prepare_gray().
	 */
	static void prepare_gray(collector &);
};

}
