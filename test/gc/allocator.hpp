
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

/**	\file test/gc/allocator.hpp
 *	\brief Declares test cases for gc::allocator.
 */

#include "../../src/gc/allocator.hpp"
#include "../master.hpp"

namespace unit_test {

/**	\class tester<gc::allocator>
 *	\brief Class with access to the interals of gc::allocator to make tests.
 */
template<>
class tester<gc::allocator>
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
	/**	\typedef allocator
	 *	\brief Shorthand for the class being tested.
	 */
	typedef gc::allocator allocator;

	/**	\typedef handle
	 *	\brief Shorthand for a pool::handle.
	 */
	typedef gc::pool::handle handle;

	/**	\fn invariants(allocator const &obj) noexcept
	 *	\brief Checks invariants for objects of the allocator class.
	 *	\param obj The object to check.
	 *	\returns Returns true if obj satisfy invariants.
	 */
	static bool invariants(allocator const &obj) noexcept;

	/**	\fn creation(collector &)
	 *	\brief Tests the construction of an allocator.
	 */
	static void creation(collector &);

	/**	\fn growth(collector &)
	 *	\brief Tests addition of pools to an allocator.
	 */
	static void growth(collector &);
};

}
