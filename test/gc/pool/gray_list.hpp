
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

/**	\file test/gc/pool/gray_list.hpp
 *	\brief Declares test cases for gc::gray_list.
 */

#include "../../master.hpp"

namespace gc {

class gray_list;

}

namespace unit_test {

/**	\class tester<gc::gray_list>
 *	\brief Class with access to the internals of gc::gray_list to make tests.
 */
template<>
class tester<gc::gray_list>
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
	/**	\typedef gray_list
	 *	\brief Shorthand for the class being tested.
	 */
	using gray_list = typename gc::gray_list;

	/**	\fn invariants(gray_list const &obj) noexcept
	 *	\brief Checks invariants for objects of the gray_list class.
	 *	\param obj The object to check.
	 *	\details Preforms assertions checking if the invariants are true.
	 */
	static void invariants(gray_list const &obj) noexcept;

	/**	\fn null_list(collector &)
	 *	\brief Tests the creation of a gray_list with a nullptr.
	 */
	static void null_list(collector &);

	/**	\fn creation(collector &)
	 *	\brief Tests the creation of a gray_list with a valid pointer.
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
