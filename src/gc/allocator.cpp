
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

/**	\file src/gc/allocator.cpp
 *	\brief Defines the implementation of gc::allocator.
 */

namespace gc {

allocator::allocator(identity const &id, std::size_t u, traits::flag_type f) :
	iallocator(), mtx(), pools(), type(id), unit(u), capacity(pool::select_capacity(u)), flags(f)
{
	collector::insert_alloc(*this);
	grow();
}

allocator::~allocator() noexcept { collector::erase_alloc(*this); }

allocator::handle &allocator::grow()
{
	handle &p = pools.emplace_front(type, capacity, unit);
	collector::insert_source(*p);
	return p;
}

void *allocator::allocate_impl()
{
	std::lock_guard<std::mutex> lock{mtx};
	for (handle &x : pools) {
		if (0 < x.available()) { return x.allocate(); }
	}
	try {								// grow() will throw std::bad_alloc if a pool could not
		return grow().allocate();		// be created or pools could not allocate a new list item.
	} catch (std::bad_alloc const &) {} // On throw, the function will continue.
	collector::wait();					// Wait on collection cycle.
	for (handle &x : pools) {			// Try again to allocate.
		if (0 < x.available()) { return x.allocate(); }
	}
	return grow().allocate();
}

void *allocator::allocate() { return allocate_impl(); }

void *allocator::allocate(std::nothrow_t) noexcept
{
	try {
		return allocate_impl();
	} catch (std::bad_alloc const &) {
		return nullptr;
	}
}

std::size_t allocator::shrink(std::size_t goal) noexcept
{
	std::size_t available{0};
	std::size_t empty{0};
	for (auto const &x : pools) {
		if (x.empty()) {
			empty++;
		} else {
			available += x.available();
		}
	}
	std::size_t const half{(pools.size() - (available / capacity)) / 2};
	goal = std::max(goal, half < empty ? empty - half : 0);
	if (empty == 0 || goal == 0) {
		return 0;
	} else {
		empty = std::min(goal, available <= capacity ? empty - 1 : empty);
		std::size_t freed{empty};
		auto pos = pools.cbegin();
		while (0 < empty) {
			SYNAFIS_ASSERT(pos != pools.cend());
			handle const &x = *pos;
			auto const cur = pos++;
			if (x.empty()) {
				collector::erase_source(*x);
				pools.erase(cur);
				empty--;
			}
		}
		return freed;
	}
}

}
