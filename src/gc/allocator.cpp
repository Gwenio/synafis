
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

#ifndef SYNAFIS_GC_IDACCESS_HPP
#include "idaccess.hpp"
#endif

/**	\file src/gc/allocator.cpp
 *	\brief Defines the implementation of gc::allocator.
 */

namespace gc {

allocator::allocator(identity const &id, std::size_t u, traits::flag_type f) :
	iallocator(), mtx(), pools(), full_begin(pools.cend()), type(id), unit(u),
	capacity(pool::select_capacity(u)), flags(f)
{
	grow();
}

allocator::~allocator() noexcept
{
	pools.sort([](handle const &x, handle const &y) -> bool {
		return std::addressof(*x) < std::addressof(*y);
	});
	erase_sources(pools.begin(), pools.end(), idaccess::has_traverser(type));
}

allocator::handle &allocator::grow()
{
	handle &p = pools.emplace_front(type, capacity, unit);
	insert_source(*p, idaccess::has_traverser(type));
	return p;
}

void allocator::move_back() noexcept
{
	pool *addr{std::addressof(*pools.front())};
	auto const target = std::find_if(full_begin, pools.cend(),
		[&addr](handle const &p) -> bool { return addr < std::addressof(*p); });
	pools.splice(target, pools, pools.cbegin());
	if (target == full_begin) { full_begin--; }
}

void *allocator::allocate_impl()
{
	std::unique_lock<std::mutex> l{mtx};
	while (full_begin != pools.cbegin()) {
		handle &ref{pools.front()};
		if (ref.full()) {
			move_back(); // Move back full pool.
		} else {		 // Allocate if room.
			return ref.allocate();
		}
	}
	try {								// grow() will throw std::bad_alloc if a pool could not
		return grow().allocate();		// be created or pools could not allocate a new list item.
	} catch (std::bad_alloc const &) {} // On throw, the function will continue.
	l.unlock();							// Unlock while waiting.
	collector::wait();					// Wait on collection cycle.
	l.lock();							// Regain lock.
	while (full_begin != pools.cbegin()) { // Try again to allocate.
		handle &ref{pools.front()};
		if (ref.full()) {
			move_back();
		} else {
			return ref.allocate();
		}
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
	// Sort pools by available slots. This puts empty pools at the front and full at the back.
	// For those of same available slots, sort by memory location.
	// This lets us send the empties to erase_sources which needs a sorted range sorted by location.
	// To minimize future sorting, pools are left as close to address order as possible.
	pools.sort([](handle const &x, handle const &y) -> bool {
		if (x.available() == y.available()) {
			return std::addressof(*x) < std::addressof(*y);
		} else if (x.empty() || y.full()) {
			return true;
		} else if (x.full() || y.empty()) {
			return false;
		} else {
			return std::addressof(*x) < std::addressof(*y);
		}
	});
	// Count empty pools and find the iterator passed the last empty pool.
	std::size_t empty{0};
	auto empty_end = pools.cbegin();
	while (empty_end != pools.cend() && empty_end->empty()) {
		empty++;
		empty_end++;
	}
	if (empty_end == pools.cbegin()) {
		return 0; // Return if no empty pools.
	} else {
		std::size_t available{0};
		full_begin = empty_end;
		while (!(full_begin == pools.cend() || full_begin->full())) {
			available += full_begin->available();
			full_begin++;
		}
		// Possibly set a new goal if there is a lot of unused slots in the pools.
		std::size_t const reserve{pools.size() - empty - (available / capacity)};
		goal = std::max(goal, reserve < empty ? empty - reserve - 1 : 0);
		if (available < capacity) {							 // Reduce empty pool count by one.
			if (--empty_end == pools.cbegin()) { return 0; } // Return if no empty pools left.
			empty--; // The calculation for reserve would remain
		}			 // the same if available is increased by capacity.
	}
	if (goal == 0) { // If goal is none still then quit.
		return 0;
	} else if (goal < empty) { // Leave some empty pools if goal is less.
		std::ptrdiff_t const back{static_cast<std::ptrdiff_t>(empty - goal)};
		if (goal < back) {
			empty_end = pools.cbegin();
			std::advance(empty_end, goal); // Advance from pools.cbegin().
		} else {
			std::advance(empty_end, -back); // Back up from the original empty_end.
		}
		empty = goal;
	}
	erase_sources(pools.cbegin(), empty_end, idaccess::has_traverser(type), std::defer_lock);
	pools.erase(pools.cbegin(), empty_end);
	return empty;
}

}
