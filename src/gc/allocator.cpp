
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

#include "allocator.hpp"

#ifndef SYNAFIS_GC_IMPL_HPP
#include "impl.hpp"
#endif

/**	\file gc/allocator.cpp
 *	\brief Defines the implementation of gc::allocator.
 */

namespace gc {

allocator::allocator(identity const&id, std::size_t u, traits::flag_type f) :
	iallocator(), mtx(), pools(), type(id), unit(u),
	capacity(pool::select_capacity(u)), flags(f) {
	grow();
}

allocator::~allocator() noexcept {}

allocator::handle &allocator::grow() {
	return pools.emplace_front(type, capacity, unit);
}

void *allocator::allocate_impl() {
	std::lock_guard<std::mutex> lock{mtx};
	for (handle &x : pools) {
		if (0 < x.available()) {
			return x.allocate();
		}
	}
	return grow().allocate();
	//! \todo If grow fails, should run GC cycle before giving up.
}

void *allocator::allocate() {
	return allocate_impl();
}

void *allocator::allocate(std::nothrow_t) noexcept {
	try {
		return allocate_impl();
	} catch(std::bad_alloc &) {
		return nullptr;
	}
}

}
