
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

#include "block.hpp"

namespace datatype {

template<>
void block::step<operation::env_swap>(state_type &state) {
	auto temp = state.current();
	//!	\todo The error below requires changes to state_type, value_type, and/or environment_type.
	state.env(*state);
	state = std::move(temp);
}

template<>
void block::step<operation::env_child>(state_type &state) {
	state = state.current();
	if (config::mutable_env) {
		state.env(state.child());
	}
}

template<>
void block::step<operation::env_fresh>(state_type &state) {
	state = state.current();
	state.env(state_type::fresh());
}

template<>
void block::step<operation::stack_push>(state_type &state) {
	state = state.top();
	state.pop();
}

template<>
void block::step<operation::stack_push>(state_type &state) {
	state.push(*state);
}

template<>
void block::step<operation::stack_swap>(state_type &state) {
	value_type temp{*state};
	state = state.top();
	state.pop();
	state.push(std::move(temp));
}

template<>
void block::step<operation::stack_cons>(state_type &state) {
	state = state_type::cons(state.top(), *state);
	state.pop();
}

template<>
void block::step<operation::stack_frame>(state_type &state) {
	state.push(state.frame());
}

template<>
block block::impl<std::tuple<operation::exec_prep,
	operation::exec_call>>(state_type &state) {
	value_type x{state.top()};
	state.pop();
	value_type y{state.top()};
	state.pop();
	state.push(*state);
	state = std::move(x);
	return state(std::move(y));
}

template<>
block block::finish<operation::exec_call>(state_type &state) {
	return state();
}

template void block::step<operation::env_swap>(state_type &);

template void block::step<operation::env_child>(state_type &);

template void block::step<operation::env_fresh>(state_type &);

template void block::step<operation::stack_pop>(state_type &);

template void block::step<operation::stack_push>(state_type &);

template void block::step<operation::stack_cons>(state_type &);

template void block::step<operation::stack_frame>(state_type &);

template block block::impl<std::tuple<operation::exec_prep,
	operation::exec_call>>(state_type &);

template block block::finish<operation::exec_call>(state_type &);

}
