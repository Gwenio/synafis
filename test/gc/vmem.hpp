
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

/**	\file test/gc/vmem.hpp
 *	\brief Declares the tester for gc::vmem.
 */

#include "../master.hpp"
#include "../../src/gc/impl/vmem.hpp"

namespace unit_test {

/**	\class tester<gc::vmem>
 *	\brief Class with access to the internals of gc::vmem to make tests.
 */
template<>
class tester<gc::vmem>
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
	/**	\typedef vmem
	 *	\brief Shorthand for the class being tested.
	 */
	using vmem = typename gc::vmem;

	/**	\fn invariants(vmem const &obj) noexcept
	 *	\brief Checks invariants for objects of the vmem class.
	 *	\param obj The object to check.
	 *	\returns Returns true if obj satisfy invariants.
	 */
	static bool invariants(vmem const &obj) noexcept;

	/**	\fn no_access(vmem const &obj, std::size_t offset, std::size_t length) noexcept
	 *	\brief Checks that an area of memory in a vmem is unaccessible.
	 *	\param obj The object to check.
	 *	\param offset The starting point in the vmem.
	 *	\param length The size of the area to check.
	 *	\returns Returns true if the memory is unaccessible.
	 */
	static bool no_access(vmem const &obj, std::size_t offset, std::size_t length) noexcept;

	/**	\fn is_readonly(vmem const &obj, std::size_t offset, std::size_t length) noexcept
	 *	\brief Checks that an area of memory in a vmem is readonly.
	 *	\param obj The object to check.
	 *	\param offset The starting point in the vmem.
	 *	\param length The size of the area to check.
	 *	\returns Returns true if the memory is readonly.
	 */
	static bool is_readonly(vmem const &obj, std::size_t offset, std::size_t length) noexcept;

	/**	\fn is_writable(vmem const &obj, std::size_t offset, std::size_t length) noexcept
	 *	\brief Checks that an area of memory in a vmem is writable.
	 *	\param obj The object to check.
	 *	\param offset The starting point in the vmem.
	 *	\param length The size of the area to check.
	 *	\returns Returns true if the memory is writable.
	 */
	static bool is_writable(vmem const &obj, std::size_t offset, std::size_t length) noexcept;

	/**	\fn is_allocated(vmem const &obj) noexcept
	 *	\brief Checks that an memory in a vmem is allocated.
	 *	\param obj The object to check.
	 *	\returns Returns true if the memory is allocated.
	 */
	static bool is_allocated(vmem const &obj) noexcept;

	/**	\fn is_free(void *addr, std::size_t length) noexcept
	 *	\brief Checks that an area of memory is free.
	 *	\param addr The start address.
	 *	\param length The size of the area.
	 *	\returns Returns true if the memory is free.
	 */
	static bool is_free(void *addr, std::size_t length) noexcept;

	/**	\fn sane_page_size(collector &)
	 *	\brief Test case checking the sanity of vmem::page_size.
	 */
	static void sane_page_size(collector &);

	/**	\fn def_init(collector &)
	 *	\brief Test case checking that the default contructor works.
	 */
	static void def_init(collector &);

	/**	\fn reg_init(collector &)
	 *	\brief Test case checking normal constructor works.
	 */
	static void reg_init(collector &);

	/**	\fn move_init(collector &)
	 *	\brief Test case checking them move constructor works.
	 */
	static void move_init(collector &);

	/**	\fn destruct(collector &)
	 *	\brief Test case checking the destructor frees memory.
	 */
	static void destruct(collector &);

	/**	\fn null_assign(collector &)
	 *	\brief Test case checking that assigning nullptr releases memory.
	 */
	static void null_assign(collector &);

	/**	\fn move_assign(collector &)
	 *	\brief Test case checking that assigning nullptr releases memory.
	 */
	static void move_assign(collector &);

	/**	\fn bool_convert(collector &)
	 *	\brief Test case checking the boolean operations.
	 */
	static void bool_convert(collector &);

	/**	\fn bounds(collector &)
	 *	\brief Test case checking the boundary access members.
	 */
	static void bounds(collector &);

	/**	\fn access(collector &)
	 *	\brief Test case checking the offset access members.
	 */
	static void access(collector &);

	/**	\fn protect(collector &)
	 *	\brief Test case checking the protections operations.
	 */
	static void protect(collector &);
};

}
