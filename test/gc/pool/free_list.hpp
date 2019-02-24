
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

/**	\file test/gc/pool/free_list.hpp
 *	\brief Declares test cases for gc::free_list.
 */

#include "../../../src/gc/impl/pool/free_list.hpp"
#include "suites.hpp"

namespace unit_test {

/**	\class tester<gc::free_list>
 *	\brief Class with access to the internals of gc::free_list to make tests.
 */
template<>
class tester<gc::free_list>
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
	/**	\typedef free_list
	 *	\brief Shorthand for the class being tested.
	 */
	using free_list = typename gc::free_list;

	/**	\typedef node
	 *	\brief Makes the node type available.
	 */
	using node = typename free_list::node;

	/**	\fn invariants(free_list const &obj, void *begin, void *end) noexcept
	 *	\brief Checks invariants for objects of the free_list class.
	 *	\param obj The object to check.
	 *	\param area The gc::arena used to construct the list being checked.
	 *	\details Preforms assertions checking if the invariants are true.
	 *	\returns Returns true if and only if invariants are true.
	 */
	static bool invariants(free_list const &obj, gc::arena const &area) noexcept;

	/**	\fn creation(collector &)
	 *	\brief Tests the constructors.
	 */
	static void creation(collector &);

	/**	\fn pushing(collector &)
	 *	\brief Tests adding addresses to the list.
	 */
	static void pushing(collector &);

	/**	\fn popping(collector &)
	 *	\brief Tests removing addresses from the list.
	 */
	static void popping(collector &);
};

}
