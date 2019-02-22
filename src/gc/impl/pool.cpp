
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

/**	\file src/gc/impl/pool.cpp
 *	\brief Defines the implementation of gc::pool.
 *	\ingroup gc_pool
 */

#include "pool.hpp"

#include "idaccess.hpp"
#include "soft_ptr_data.hpp"
#include "core.hpp"

#include <algorithm>

static_assert(sizeof(gc::pool) <= gc::bitmap::placement(sizeof(gc::pool)),
	"bitmap::placement(sizeof(pool)) must be larger than the size of a pool.");

static_assert(gc::bitmap::placement(sizeof(gc::pool)) % alignof(gc::bitmap::group) == 0,
	"bitmap::placement(sizeof(pool)) must meet the alignment requirement of bitmap::group.");

namespace gc {

pool::pool(identity const &id, blueprint const &cfg) :
	pool(id, cfg, std::move(cfg.prepare_region()))
{}

pool::pool(identity const &id, blueprint const &cfg, vmem &&mem) noexcept :
	pool(id, std::forward<vmem>(mem), arena{cfg.capacity, cfg.unit, mem[cfg.begin], cfg.length},
		cfg.prepare_gray(id, mem), mem[cfg.maps])
{}

pool::pool(identity const &id, vmem &&m, arena const &s, void **g, void *b) noexcept :
	isource(), region(std::forward<vmem>(m)), type(id), store(s), tracking(), gray(g), free(s)
{
	// Sanity check the location of the arena.
	SYNAFIS_ASSERT(region.begin() <= store.cbegin() && store.cend() <= region.end());
	bit_group *const maps{static_cast<bit_group *>(b)};
	initialized = bitmap{maps};
	// Set the colors pointer. bitmap and colors should be a single array split into halves.
	std::size_t const bit_array_half{bitmap::length(store.max())};
	reachable = bitmap{maps + bit_array_half};
	// End of bitmaps.
	bit_group *const last{maps + (bit_array_half * 2)};
	// Sanity check the location of bitmap and colors.
	SYNAFIS_ASSERT(region.begin() <= maps && last <= region.end());
	SYNAFIS_ASSERT(last <= store.cbegin() || store.cend() <= maps);
	// Set all bits in the bitmap to false and set all bits for colors to white (false).
	for (bit_group *current = maps; current < last; current++) {
		current->reset();
	}
}

pool::~pool() noexcept
{
	for (auto *cur : tracking) {
		*cur = nullptr;
	}
	if (idaccess::has_finalizer(type) && !free.full()) {
		std::size_t bit{0};
		bit_group *batch{initialized.begin()};
		for (void *current = store.begin(); current < store.end();
			 current = add_offset(current, store.size())) {
			if (batch->test(bit)) { idaccess::finalize(type, current); }
			if (++bit == bitmap::bits()) {
				bit = 0;
				batch++;
			}
		}
	}
}

void pool::deallocate(void *ptr) noexcept
{
	SYNAFIS_ASSERT(from(ptr));
	ptr = base_of(ptr);
	idaccess::finalize(type, ptr);
	free.push(ptr);
}

void *pool::allocate() noexcept
{
	if (free.full()) {
		return nullptr;
	} else {
		void *addr{free.pop()};
		// Mark as initialized.
		std::size_t const offset{store.get_slot(addr)};
		SYNAFIS_ASSERT(offset < store.max());
		SYNAFIS_ASSERT(!initialized.test(offset)); // Assert that it is not being marked twice.
		initialized.set(offset);
		SYNAFIS_ASSERT(from(addr));
		return addr;
	}
}

void pool::discarded(void *addr) noexcept
{
	SYNAFIS_ASSERT(from(addr));
	SYNAFIS_ASSERT(sub_addr(addr, store.begin()) % store.size() == 0);
	std::size_t const offset{store.get_slot(addr)};
	SYNAFIS_ASSERT(offset < store.max());
	SYNAFIS_ASSERT(initialized.test(offset)); // Assert that the slot is marked as allocated.
	initialized.reset(offset);
	SYNAFIS_ASSERT(addr == store[offset]);
	free.push(addr);
}

void *pool::base_of(void *ptr) const noexcept
{
	SYNAFIS_ASSERT(from(ptr));
	auto const x = sub_addr(ptr, store.cbegin());
	if (0 < x) {
		return sub_offset(ptr, x % store.size());
	} else {
		return ptr; // In the case x == slots, this is correct.
	}
}

void pool::mark(void *ptr) noexcept
{
	SYNAFIS_ASSERT(from(ptr));
	std::size_t const offset{store.get_slot(ptr)};
	SYNAFIS_ASSERT(offset < store.max());
	ptr = base_of(ptr);
	// Assert that the slot is initialized.
	SYNAFIS_ASSERT(initialized.test(offset));
	if (!reachable.test(offset)) {
		if (gray) {
			SYNAFIS_ASSERT(pending() < store.max());
			gray.push(ptr);
		}
		reachable.set(offset);
	}
}

bool pool::traverse(void *data, enumerate_cb cb) noexcept
{
	SYNAFIS_ASSERT(idaccess::has_traverser(type));
	if (has_pending()) {
		do {
			void *slot{gray.pop()};
			SYNAFIS_ASSERT(from(slot));
			SYNAFIS_ASSERT(initialized.test(store.get_slot(slot)));
			SYNAFIS_ASSERT(reachable.test(store.get_slot(slot)));
			idaccess::traverse(type, slot, data, cb);
		} while (has_pending());
		return true;
	} else {
		return false;
	}
}

void pool::sweep() noexcept
{
	bit_group *alloc{initialized.begin()};
	bit_group const *marks{reachable.cbegin()};
	void *current{store.begin()};
	std::size_t const skip{store.size() * bitmap::bits()};
	auto start = tracking.begin();
	do {
		// Only bits for slots that are allocated and unmarked will be true.
		bit_group const group{*alloc ^ *marks};
		if (group.any()) {
			for (std::size_t bit = 0; bit < bitmap::bits(); bit++) {
				if (group[bit]) {
					SYNAFIS_ASSERT(alloc->test(bit));
					SYNAFIS_ASSERT(!(marks->test(bit)));
					auto pos = std::find_if(start, tracking.end(),
						[&current](soft_ptr::data *cur) -> bool { return current <= cur->get(); });
					if (pos != tracking.end()) {
						if ((*pos)->get() == current) {
							auto *soft = *pos;
							start = tracking.erase(pos);
							*soft = nullptr;
						} else {
							start = pos; // All future items will be greater than the current.
						}
					}
					deallocate(current);
				}
				current = add_offset(current, store.size());
				if (store.end() <= current) { break; }
			}
		} else {
			// Skip group, all are marked or unallocated.
			current = add_offset(current, skip);
		}
		alloc->reset();
		alloc++;
		marks++;
	} while (current < store.end());
	// The black slots are the new allocated set.
	std::swap(initialized, reachable);
}

soft_ptr::data *pool::fetch(void *ptr) noexcept
{
	SYNAFIS_ASSERT(from(ptr));
	auto pos = std::find_if(tracking.begin(), tracking.end(),
		[ptr](soft_ptr::data *cur) -> bool { return ptr <= cur->get(); });
	if (pos != tracking.end() && (*pos)->get() == ptr) {
		return *pos;
	} else {
		try {
			pos = tracking.insert(pos, static_cast<soft_ptr::data *>(nullptr));
			*pos = soft_ptr::data::create(ptr);
			return *pos;
		} catch (std::bad_alloc const &) {}
		collector::wait();
		pos = tracking.insert(pos, static_cast<soft_ptr::data *>(nullptr));
		*pos = soft_ptr::data::create(ptr);
		return *pos;
	}
}

}
