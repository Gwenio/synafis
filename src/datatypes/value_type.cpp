
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


/**	\class traverse_visitor
 *	\brief Type for visiting store for garbage collector traversal.
 */
class traverse_visitor {
private:
	void *arg;
	gc::enumerate_cb cb;
public:
	traverse_visitor() = delete;
	constexpr traverse_visitor(void *a, gc::enumerate_cb c) noexcept :
		arg(a), cb(c) {}
	~traverse_visitor() noexcept = default;
	void operator()(gc::hard_ptr const&ptr) const noexcept {
		ptr.traverse(arg, cb);
	}
	template<typename T>
	void operator()(T const&) const noexcept {}
};

/**	\class remap_visitor
 *	\brief Type for visiting store for garbage collector remapping.
 */
class remap_visitor {
private:
	void *arg;
	gc::remap_cb cb;
public:
	remap_visitor() = delete;
	constexpr remap_visitor(void *a, gc::remap_cb c) noexcept :
		arg(a), cb(c) {}
	~remap_visitor() noexcept = default;
	void operator()(gc::hard_ptr &ptr) const noexcept {
		ptr.remap(arg, cb);
	}
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
