
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

#include "state_type.hpp"

namespace datatype {

state_type::state_type() {
	gc::scoped_lock l{};
	gc::root::register_(*this);
}

state_type::~state_type() {
	gc::root::unregister(*this);
}

block state_type::operator()() {
	std::tuple<bool, block> temp{call(top())};
	if (!config::keep_call && std::get<0>(temp)) {
		pop();
	}
	return std::get<1>(temp);
}

block state_type::operator()(value_type value) {
	std::tuple<bool, block> temp{call(value)};
	if (config::keep_call && std::get<0>(temp)) {
		push(value);
	}
	return std::get<1>(temp);
}

void state_type::traverse(void *data, gc::enumerate_cb cb) const noexcept {
	accumulator.traverse(data, cb);
	(*cb)(data, static_cast<void *>(environment));
}
void state_type::remap(void *data, gc::remap_cb cb) noexcept {
	;
}

}
