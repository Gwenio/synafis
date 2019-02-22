
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

#include "allocator.hpp"
#include "core.hpp"
#include "idaccess.hpp"

/**	\file src/gc/impl/allocator.cpp
 *	\brief Defines the implementation of gc::allocator.
 *	\ingroup gc_impl
 */

using gc::allocator;
using gc::pool;
using pool_list = allocator::pool_list;

namespace {

/**	\fn is_sorted_pools(pool_list const &l)
 *	\brief Checks if a list of pools are sorted.
 *	\param l The list to check.
 *	\returns Returns true if the pools in 'l' are sorted.
 */
bool is_sorted_pools(pool_list const &l)
{
	return std::is_sorted(l.cbegin(), l.cend(), [](pool const &x, pool const &y) { return x < y; });
}

/**	\fn partition_pools(pool_list &from, Cond &cond)
 *	\brief Removes pools matching a condition and places them in a new list preserving their order.
 *	\tparam Cond The type for cond.
 *	\param from The list to partition.
 *	\param cond A function taking 'pool const &' as a parameter and returning bool.
 *	\returns Returns a list containing all pools cond returned true for.
 */
template<typename Cond>
pool_list partition_pools(pool_list &from, Cond &cond)
{
	pool_list temp{};
	for (auto cur = from.cbegin(); cur != from.cend();) {
		if (cond(*cur)) {
			auto it = cur++;
			temp.splice(temp.cend(), from, it);
		} else {
			cur++;
		}
	}
	return std::move(temp);
}

}

namespace gc {

allocator::allocator(identity const &id, std::size_t u, traits::flag_type f) :
	iallocator(), mtx(), empty_pools(), part_pools(), full_pools(), type(id), cfg(blueprint(id, u))
{
	grow();
}

allocator::~allocator() noexcept
{
	full_pools.merge(std::move(empty_pools));
	full_pools.merge(std::move(part_pools));
	SYNAFIS_ASSERT(is_sorted_pools(full_pools));
	erase_sources(full_pools.begin(), full_pools.end(), idaccess::has_traverser(type));
}

pool &allocator::grow()
{
	pool &p{empty_pools.emplace_front(type, cfg)};
	insert_source(p, idaccess::has_traverser(type));
	current = empty_pools.begin();
	return p;
}

void allocator::move_back() noexcept
{
	SYNAFIS_ASSERT(current->full());
	pool const &ref = *current;
	auto const pos = std::find_if(full_pools.cbegin(), full_pools.cend(),
		[&ref](pool const &cur) -> bool { return cur < ref; });
	if (current == empty_pools.begin()) {
		full_pools.splice(pos, empty_pools, current);
	} else if (current != pos) {
		SYNAFIS_ASSERT(current == part_pools.begin());
		full_pools.splice(pos, part_pools, current);
	}
}

void *allocator::try_allocate()
{
	if (current->full()) {
		move_back();
		if (!empty_pools.empty()) {
			current = empty_pools.begin();
		} else if (!part_pools.empty()) {
			current = part_pools.begin();
		} else {
			return grow().allocate();
		}
	}
	SYNAFIS_ASSERT(!current->full());
	return current->allocate();
}

void *allocator::allocate_impl()
{
	std::unique_lock<std::mutex> l{mtx};
	try {								// grow() will throw std::bad_alloc if a pool could not
		return try_allocate();			// be created or pools could not allocate a new list item.
	} catch (std::bad_alloc const &) {} // On throw, the function will continue.
	l.unlock();							// Unlock while waiting.
	collector::wait();					// Wait on collection cycle.
	l.lock();							// Regain lock.
	return try_allocate();				// Try again, if still failure then the allocation fails.
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

void allocator::discarded(void *addr) noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	if (current->from(addr)) {
		current->discarded(addr);
	} else {
		auto const it = std::find_if(full_pools.begin(), full_pools.end(),
			[addr](pool const &cur) { return cur.location() < addr; });
		if (it->from(addr)) {
			it->discarded(addr);
			return;
		} else {
			SYNAFIS_FAILURE("The pool for the address being discarded was not found.");
		}
	}
}

std::size_t allocator::shrink(std::size_t goal) noexcept
{
	std::lock_guard<std::mutex> l{mtx};
	// Sort pools by available slots. This puts empty pools at the front and full at the back.
	// For those of same available slots, sort by memory location.
	// This lets us send the empties to erase_sources which needs a sorted range sorted by location.
	// To minimize future sorting, pools are left as close to address order as possible.
	{
		auto const is_empty = [](pool const &p) -> bool { return p.empty(); };
		pool_list empty{std::move(partition_pools(part_pools, is_empty))};
		pool_list part{std::move(
			partition_pools(full_pools, [](pool const &p) -> bool { return !p.full(); }))};
		empty.merge(std::move(partition_pools(part, is_empty)));
		if (current->full()) {
			move_back();
		} else if (!current->empty() && current == empty_pools.begin()) {
			pool const &ref = *current;
			pool_list &list{part_pools.size() <= part.size() ? part_pools : part};
			auto const it = std::find_if(
				list.cbegin(), list.cend(), [&ref](pool const &cur) -> bool { return cur < ref; });
			list.splice(it, empty_pools, empty_pools.cbegin());
		}
		empty_pools.merge(std::move(empty));
		part_pools.merge(std::move(part));
	}
	SYNAFIS_ASSERT(is_sorted_pools(empty_pools));
	SYNAFIS_ASSERT(std::all_of(
		empty_pools.cbegin(), empty_pools.cend(), [](pool const &x) { return x.empty(); }));
	SYNAFIS_ASSERT(is_sorted_pools(part_pools));
	SYNAFIS_ASSERT(std::none_of(part_pools.cbegin(), part_pools.cend(),
		[](pool const &x) { return x.empty() || x.full(); }));
	SYNAFIS_ASSERT(is_sorted_pools(full_pools));
	SYNAFIS_ASSERT(std::all_of(
		full_pools.cbegin(), full_pools.cend(), [](pool const &x) { return x.full(); }));
	std::size_t release{empty_pools.size()};
	std::size_t available{0};
	for (pool const &cur : part_pools) {
		available += cur.available();
	}
	// Count empty pools and find the iterator passed the last empty pool.
	if (available < cfg.capacity && 0 < release) { release--; }
	if (release == 0) {
		if (!empty_pools.empty()) {
			current = empty_pools.begin();
		} else if (!part_pools.empty()) {
			current = part_pools.begin();
		} else {
			SYNAFIS_ASSERT(!full_pools.empty());
			current = full_pools.begin();
		}
		return 0; // Return if no empty pools.
	}
	{
		// Possibly set a new goal if there is a lot of unused slots in the pools.
		std::size_t const used{full_pools.size() + part_pools.size()};
		std::size_t const free{release + (available / cfg.capacity)};
		goal = std::max(goal, used <= free ? std::min(release, free - used) : 0);
	}
	if (goal == 0) { // If goal is none still then quit.
		if (!empty_pools.empty()) {
			current = empty_pools.begin();
		} else if (!part_pools.empty()) {
			current = part_pools.begin();
		} else {
			SYNAFIS_ASSERT(!full_pools.empty());
			current = full_pools.begin();
		}
		return 0;
	}
	auto release_begin = empty_pools.cbegin();
	release = std::min(goal, release);
	std::size_t const back{empty_pools.size() - goal};
	if (goal < back) {
		auto it = std::make_reverse_iterator(empty_pools.cend());
		std::advance(it, goal);	// Advance from empty_pools.cend().
		release_begin = it.base(); // Return to normal.
	} else {
		std::advance(release_begin, back); // Back up from the original empty_end.
	}
	SYNAFIS_ASSERT(std::distance(release_begin, empty_pools.cend()) == release);
	erase_sources(
		release_begin, empty_pools.cend(), idaccess::has_traverser(type), std::defer_lock);
	empty_pools.erase(release_begin, empty_pools.cend());
	if (!empty_pools.empty()) {
		current = empty_pools.begin();
	} else if (!part_pools.empty()) {
		current = part_pools.begin();
	} else {
		SYNAFIS_ASSERT(!full_pools.empty());
		current = full_pools.begin();
	}
	return release;
}

}
