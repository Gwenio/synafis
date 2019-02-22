
/*
ISC License (ISC)

Copyright 2018-2019 Adam Armstrong

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

/**	\file test/gc/pool.hpp
 *	\brief Declares test cases for gc::pool.
 */

#include "../../src/gc/impl/pool.hpp"
#include "../master.hpp"

namespace unit_test {

/**	\class tester<::gc::pool>
 *	\brief Class with access to the internals of gc::pool to make tests.
 */
template<>
class tester<::gc::pool>
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
	/**	\typedef pool
	 *	\brief Shorthand for the class being tested.
	 */
	typedef ::gc::pool pool;

	/**	\fn invariants(pool const &obj) noexcept
	 *	\brief Checks invariants for objects of the pool class.
	 *	\param obj The object to check.
	 *	\details Preforms assertions checking if the invariants are true.
	 */
	static void invariants(pool const &obj) noexcept;

	/**	\fn creation(collector &)
	 *	\brief Tests the creation of a pool.
	 */
	static void creation(collector &);

	/**	\fn destruction(collector &)
	 *	\brief Tests that pools properly free their virtual memory.
	 */
	static void destruction(collector &);

	/**	\fn allocation(collector &)
	 *	\brief Tests the ability to allocate objects from the pool.
	 */
	static void allocation(collector &);

	/**	\fn ownership(collector &)
	 *	\brief Tests the ability to identify the pool that allocated a pointer.
	 */
	static void ownership(collector &);

	/**	\fn sweeping(collector &)
	 *	\brief Tests the ability to mark and sweep objects.
	 */
	static void sweeping(collector &);

	/**	\fn discarding(collector &)
	 *	\brief Tests the ability to discard objects that failed to be initialized.
	 */
	static void discarding(collector &);

	/**	\fn traversing(collector &)
	 *	\brief Tests the ability to traverse objects in a pool to mark reachable objects.
	 */
	static void traversing(collector &);
};

}
