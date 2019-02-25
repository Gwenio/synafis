
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

/**	\file test/gc/collector.hpp
 *	\brief Declares the tester for gc::collector.
 */

#include "../master.hpp"

namespace gc {

class collector;

}

namespace unit_test {

/**	\class tester<gc::collector>
 *	\brief Class with access to the internals of ::gc::collector to make tests.
 */
template<>
class tester<gc::collector>
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
	/**	\typedef collector
	 *	\brief Shorthand for the class being tested.
	 */
	using collector = typename gc::collector;

	/**	\fn no_readers() noexcept
	 *	\brief Checks that the collector singleton has no readers.
	 *	\returns Returns true if there are no readers holding the collector lock.
	 */
	static bool no_readers() noexcept;

	/**	\fn has_readers() noexcept
	 *	\brief Checks that the collector singleton has readers.
	 *	\returns Returns true if there are readers holding the collector lock.
	 */
	static bool has_readers() noexcept;
};

}
