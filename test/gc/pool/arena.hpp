
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

/**	\file test/gc/pool/arena.hpp
 *	\brief Declares test cases for gc::arena.
 */

#include "../../master.hpp"

namespace gc {

class arena;

}

namespace unit_test {

/**	\class tester<gc::arena>
 *	\brief Class with access to the internals of gc::arena to make tests.
 */
template<>
class tester<gc::arena>
{
	/**	@fn tester()
	 *	@brief Deleted.
	 */
	tester() = delete;

	/**	\fn ~tester()
	 *	\brief Deleted.
	 */
	~tester() = delete;

public:
	/**	\typedef arena
	 *	\brief Shorthand for the class being tested.
	 */
	using arena = typename gc::arena;

	/**	\fn creation(collector &)
	 *	\brief Tests the constructors.
	 */
	static void creation(collector &);

	/**	\fn ownership(collector &)
	 *	\brief Tests the ability to identify the arena that an address is in.
	 */
	static void ownership(collector &);

	/**	\fn accessing(collector &)
	 *	\brief Tests accessing specific slots in an arena.
	 */
	static void accessing(collector &);

	/**	\fn offsets(collector &)
	 *	\brief Tests determining the offset of the slot a pointer refers to.
	 */
	static void offsets(collector &);
};

}
