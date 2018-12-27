
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

#include "value_type.hpp"

namespace datatype {

class value_type::traverse_visitor {
private:
	/**	\var arg
	 *	\brief The argument passed to value_type::traverse().
	 */
	void *arg;

	/**	\var cb
	 *	\brief The callback passed to value_type::traverse().
	 */
	gc::enumerate_cb cb;
public:
	/**	\fn traverse_visitor()
	 *	\brief Deleted.
	 */
	traverse_visitor() = delete;

	/**	\fn traverse_visitor(void *a, gc::enumerate_cb c)
	 *	\brief Constructor.
	 *	\param a The value for arg.
	 *	\param c The value for cb.
	 */
	constexpr traverse_visitor(void *a, gc::enumerate_cb c) noexcept :
		arg(a), cb(c) {}

	/**	\fn ~traverse_visitor() noexcept
	 *	\brief Default.
	 */
	~traverse_visitor() noexcept = default;

	/**	\fn operator()(gc::hard_ptr const&ptr) const noexcept
	 *	\brief Overload for when store contains a gc::hard_pointer.
	 *	\param ptr A reference to the gc::hard_ptr in store.
	 */
	void operator()(gc::hard_ptr const&ptr) const noexcept {
		ptr.traverse(arg, cb);
	}

	/**	\fn operator()(T const&) const noexcept
	 *	\brief All types not overloaded will have a noop definition.
	 *	\tparam T The type being visited.
	 */
	template<typename T>
	void operator()(T const&) const noexcept {}
};

class value_type::remap_visitor {
private:
	/**	\var arg
	 *	\brief The argument passed to value_type::remap().
	 */
	void *arg;

	/**	\var cb
	 *	\brief The callback passed to value_type::remap().
	 */
	gc::remap_cb cb;
public:
	/**	\fn remap_visitor()
	 *	\brief Deleted.
	 */
	remap_visitor() = delete;

	/**	\fn remap_visitor(void *a, gc::remap_cb c)
	 *	\brief Constructor.
	 *	\param a The value for arg.
	 *	\param c The value for cb.
	 */
	constexpr remap_visitor(void *a, gc::remap_cb c) noexcept :
		arg(a), cb(c) {}

	/**	\fn ~remap_visitor() noexcept
	 *	\brief Default.
	 */
	~remap_visitor() noexcept = default;

	/**	\fn operator()(gc::hard_ptr &ptr) const noexcept
	 *	\brief Overload for when store contains a gc::hard_pointer.
	 *	\param ptr A reference to the gc::hard_ptr in store.
	 */
	void operator()(gc::hard_ptr &ptr) const noexcept {
		ptr.remap(arg, cb);
	}

	/**	\fn operator()(T const&) const noexcept
	 *	\brief All types not overloaded will have a noop definition.
	 *	\tparam T The type being visited.
	 */
	template<typename T>
	void operator()(T const&) const noexcept {}
};

void value_type::traverse(void *arg, gc::enumerate_cb cb) const noexcept {
	std::visit(traverse_visitor{arg, cb}, store);
}
void value_type::remap(void *arg, gc::remap_cb cb) noexcept {
	std::visit(remap_visitor{arg, cb}, store);
}

}
