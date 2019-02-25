
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

/**	\file test/gc/pool/bitmap.hpp
 *	\brief Declares test cases for gc::bitmap.
 */

#include "../../master.hpp"

namespace gc {

class bitmap;

}

namespace unit_test {

/**	\class tester<gc::bitmap>
 *	\brief Class with access to the internals of gc::bitmap to make tests.
 */
template<>
class tester<gc::bitmap>
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
	/**	\typedef bitmap
	 *	\brief Shorthand for the class being tested.
	 */
	using bitmap = typename gc::bitmap;

	/**	\fn creation(collector &)
	 *	\brief Tests the constructor(s).
	 */
	static void creation(collector &);

	/**	\fn swapping(collector &)
	 *	\brief Tests swapping bitmaps.
	 */
	static void swapping(collector &);

	/**	\fn checking(collector &)
	 *	\brief Tests bitmap::test().
	 */
	static void checking(collector &);

	/**	\fn setting(collector &)
	 *	\brief Tests bitmap::set().
	 */
	static void setting(collector &);

	/**	\fn resetting(collector &)
	 *	\brief Tests bitmap::reset().
	 */
	static void resetting(collector &);
};

}
