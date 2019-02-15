
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

#include "pool.hpp"

/**	\file src/gc/pool.cpp
 *	\brief Defines the implementation of gc::pool.
 */

#include "impl/idaccess.hpp"
#include "soft_ptr_data.hpp"
#include "impl.hpp"

#include <algorithm>

namespace {

/**	\var bit_group_size
 *	\brief Short hand for the number of bits in gc::pool::bit_group.
 *	\invariant gc::pool::bit_group == std::bitmap<bit_group_size>
 */
inline constexpr auto const bit_group_size = gc::pool::bit_group{}.size();

static_assert(std::is_same_v<gc::pool::bit_group, std::bitset<bit_group_size>>,
	"gc::pool::bit_group and std::bitset<bit_group_size> must be the same type.");

/**	\var bitmap_offset
 *	\brief The fixed offset to a pool's bitmap.
 *	\details It is an offset from the beginning of the virtual memory the pool is in.
 *	\note Offset varies based on whether guard pages are being used.
 *	\invariant sizeof(pool) <= bitmap_offset
 *	\invariant bitmap_offset % alignof(gc::pool::bit_group) == 0
 */
inline constexpr auto const bitmap_offset =
	((sizeof(gc::pool) % alignof(gc::pool::bit_group) == 0) ?
			sizeof(gc::pool) :
			((sizeof(gc::pool) / alignof(gc::pool::bit_group)) + 1) * alignof(gc::pool::bit_group));

static_assert(
	sizeof(gc::pool) <= bitmap_offset, "bitmap_offset must be larger than the size of a pool.");

static_assert(bitmap_offset % alignof(gc::pool::bit_group) == 0,
	"bitmap_offset must meet the alignment requirement of gc::pool::bit::group.");

/**	\var bit_group_unit
 *	\brief The unit size of a gc::pool::bit_group.
 */
inline constexpr auto const bit_group_unit = gc::idaccess::unit_size<gc::pool::bit_group>();

/**	\var gray_unit
 *	\brief The unit size of a pointer to a void pointer used for the gray stack.
 */
inline constexpr auto const gray_unit = gc::idaccess::unit_size<void **>();

/**	\fn bitmap_length(std::size_t capacity) noexcept
 *	\brief The number of elements in the arrays for bitmap and colors.
 *	\param capacity The number of slots managed by the pool.
 *	\returns Returns the number of elements in bitmap and colors.
 */
inline constexpr std::size_t bitmap_length(std::size_t capacity) noexcept
{
	return (capacity / bit_group_size) + (capacity % bit_group_size == 0 ? 0 : 1);
}

/**	\fn gcd(std::size_t x, std::size_t y) noexcept
 *	\brief Calculates the greatest common divisor.
 *	\param x First value.
 *	\param y Second value.
 *	\returns Returns the greatest common divisor of x and y.
 *	\pre Written to assume x and y are both greater than zero.
 *	\pre That case will not be tested.
 *	\note It saves one iteration if y <= x.
 */
inline constexpr std::size_t gcd(std::size_t x, std::size_t y) noexcept
{
	do {
		std::size_t z{x % y};
		x = y;
		y = z;
	} while (y != 0);
	return x;
}

static_assert(gcd(180, 48) == 12 && gcd(48, 180) == 12,
	"gcd(180, 48) or with the inputs swapped should equal 12.");

}

namespace gc {

pool::pool(vmem &&mem, identity const &id, std::size_t cap, std::size_t u, void **g,
	void *start) noexcept :
	isource(),
	region(std::forward<vmem>(mem)), type(id), tracking(), capacity(cap), space(cap), unit(u),
	free(nullptr), sentinel(g), gray(g), slots(start), end(add_offset(start, cap * u))
{
	// The bitmap is always located bitmap_offset from the start of the virtual memory.
	bitmap = reinterpret_cast<bit_group *>(
		region[bitmap_offset + (config::guard_pages ? vmem::page_size : 0)]);
	// Set the colors pointer. bitmap and colors should be a single array split into halves.
	std::size_t const bit_array_half{bitmap_length(cap)};
	colors = bitmap + bit_array_half;
	// End of colors.
	bit_group *const last = bitmap + (bit_array_half * 2);
	// Sanity check the location of bitmap and colors.
	SYNAFIS_ASSERT(region.begin() <= bitmap && last <= region.end());
	SYNAFIS_ASSERT(last <= slots || end <= bitmap);
	// Set all bits in the bitmap to false and set all bits for colors to white (false).
	for (bit_group *current = bitmap; current < last; current++) {
		current->reset();
	}
	// Sanity check the location of slots and end.
	SYNAFIS_ASSERT(region.begin() <= slots && end <= region.end());
	// Place all slots in the free stack.
	void *addr{slots};
	free = reinterpret_cast<node *>(addr);
	node *current = free;
	do {
		SYNAFIS_ASSERT(from(addr));
		// Advance the address.
		addr = add_offset(addr, unit);
		if (addr < end) {
			// Set the next free slot.
			current->next = reinterpret_cast<node *>(addr);
			current = current->next;
		} else {
			// There are no more free slots, set nullptr and exit loop.
			current->next = nullptr;
			break;
		}
	} while (true);
	SYNAFIS_ASSERT(end == addr);
}

pool::~pool() noexcept
{
	for (auto *cur : tracking) {
		*cur = nullptr;
	}
	if (idaccess::has_finalizer(type) && space < capacity) {
		std::size_t bit{0};
		bit_group *batch{bitmap};
		for (void *current = slots; current < end; current = add_offset(current, unit)) {
			if (batch->test(bit)) { idaccess::finalize(type, current); }
			if (++bit == bit_group_size) {
				bit = 0;
				batch++;
			}
		}
	}
}

std::size_t pool::select_capacity(std::size_t unit) noexcept
{
	// The unit size of slots must be able to hold a node pointer.
	SYNAFIS_ASSERT(min_unit <= unit);
	// Check if the minimum object can fit in the maximum number of pages.
	if (unit * config::min_pool < config::max_pool * vmem::page_size) {
		// We start with the smallest amount that will completely fill all pages used.
		std::size_t capacity{vmem::page_size / gcd(vmem::page_size, unit)};
		// Get the memory required. This is equal to the least common multiple of unit and page_size.
		std::size_t size{capacity * unit};
		SYNAFIS_ASSERT(size % vmem::page_size == 0);
		// Ensure we do no exceed config::max_pool pages.
		if (size <= config::max_pool * vmem::page_size) {
			capacity = config::max_pool * vmem::page_size / unit;
		} else {
			// See if we can fit more of the ideal capacity in without exceeding max.
			// Both sides of the division are multiples of vmem::page_size.
			std::size_t part{(config::max_pool * vmem::page_size) / size};
			if (2 <= part) { capacity *= part; }
		}
		SYNAFIS_ASSERT(config::min_pool <= capacity);
		return capacity;
	} else {
		// When config::min_pool is the optimal capacity, this branch is simpler.
		// Capacity must always be at least config::min_pool, even if it exceeds max_pool pages.
		std::size_t capacity{config::min_pool};
		// Fill up the last occupied page as much as possible.
		std::size_t size{capacity * unit};
		capacity += (vmem::page_size - (size % vmem::page_size)) / unit;
		return capacity;
	}
}

pool::handle::handle(identity const &id, std::size_t capacity, std::size_t unit) : handle()
{
	// The unit size of slots must be able to hold a node pointer.
	SYNAFIS_ASSERT(pool::min_unit <= unit);
	std::size_t const guard{(config::guard_pages ? vmem::page_size : 0)};
	//	Calculate size rounded up to the nearest multiple of vmem::page_size.
	std::size_t size = capacity * unit;
	size += vmem::page_size - (size % vmem::page_size);
	// Calculate the offset to the end of bit groups.
	std::size_t offset{guard + bitmap_offset + (bitmap_length(capacity) * bit_group_unit * 2)};
	// Calculate gray stack beginning and end if needed.
	std::size_t gray_offset{0};
	if (idaccess::has_traverser(id)) {
		// Calculate needed alignment adjustment if needed.
		std::size_t const gray_rem{offset % alignof(void **)};
		if (0 < gray_rem) { offset += alignof(void **) - gray_rem; }
		gray_offset = offset;
		// Calculate end of gray stack.
		offset += gray_unit * capacity;
	}
	// Make offset be at the start of a page.
	offset += guard + vmem::page_size - (offset % vmem::page_size);
	vmem mem{offset + size + guard, !config::guard_pages};
	if (mem) {
		// Set up accessible areas if guard pages are used.
		if (config::guard_pages) {
			mem.writable(vmem::page_size, offset - (vmem::page_size * 2));
			mem.writable(offset, size);
		}
		void **gray{idaccess::has_traverser(id) ? static_cast<void **>(mem[gray_offset]) :
												  static_cast<void **>(nullptr)};
		ptr = new (mem[guard]) pool(std::move(mem), id, capacity, unit, gray, mem[offset]);
	} else {
		throw std::bad_alloc{};
	}
}

std::tuple<std::size_t, std::size_t> pool::bit_locate(void *ptr) const noexcept
{
	std::size_t const offset{sub_addr(ptr, slots) / unit};
	std::size_t const bit{offset % bit_group_size};
	std::size_t const group{offset / bit_group_size};
	return std::tuple<std::size_t, std::size_t>{group, bit};
}

void pool::deallocate(void *ptr) noexcept
{
	SYNAFIS_ASSERT(from(ptr));
	SYNAFIS_ASSERT(sub_addr(ptr, slots) % unit == 0);
	idaccess::finalize(type, ptr);
	node *temp{static_cast<node *>(ptr)};
	temp->next = free;
	free = temp;
	space++;
}

void *pool::allocate() noexcept
{
	if (free) {
		space--;
		void *addr{static_cast<void *>(std::exchange(free, free->next))};
		// Mark as initialized.
		std::size_t bit, group;
		std::tie(group, bit) = bit_locate(addr);
		SYNAFIS_ASSERT(!bitmap[group].test(bit)); // Assert that it is not being marked twice.
		bitmap[group].set(bit);
		return addr;
	} else {
		return nullptr;
	}
}

void pool::discarded(void *addr) noexcept
{
	SYNAFIS_ASSERT(from(addr));
	SYNAFIS_ASSERT(sub_addr(addr, slots) % unit == 0);
	std::size_t bit, group;
	std::tie(group, bit) = bit_locate(addr);
	SYNAFIS_ASSERT(bitmap[group].test(bit)); // Assert that the slot is marked as allocated.
	bitmap[group].reset(bit);
	node *temp{static_cast<node *>(addr)};
	temp->next = free;
	free = temp;
	space++;
}

void *pool::base_of(void *ptr) const noexcept
{
	SYNAFIS_ASSERT(from(ptr));
	auto const x = sub_addr(ptr, slots);
	if (0 < x) {
		return sub_offset(ptr, x % unit);
	} else {
		return ptr; // In the case x == slots, this is correct.
	}
}

void pool::mark(void *ptr) noexcept
{
	SYNAFIS_ASSERT(from(ptr));
	std::size_t bit, group;
	std::tie(group, bit) = bit_locate(ptr);
	// Assert that the slot is initialized.
	SYNAFIS_ASSERT(bitmap[group].test(bit));
	auto &ref = colors[group];
	if (gray && !ref[bit]) {
		SYNAFIS_ASSERT(sub_addr(gray, const_cast<void **>(sentinel)) < capacity);
		*gray = ptr;
		gray++;
	}
	ref.set(bit);
}

bool pool::traverse(void *data, enumerate_cb cb) noexcept
{
	if (has_pending()) {
		do {
			gray--;
			void *obj{*gray};
			idaccess::traverse(type, obj, data, cb);
		} while (has_pending());
		return true;
	} else {
		return false;
	}
}

void pool::sweep() noexcept
{
	bit_group *alloc{bitmap};
	bit_group *marks{colors};
	void *current{slots};
	auto start = tracking.begin();
	do {
		// Only bits for slots that are allocated and unmarked will be true.
		bit_group const group{*alloc ^ *marks};
		if (group.any()) {
			for (std::size_t bit = 0; bit < bit_group_size; bit++) {
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
				current = add_offset(current, unit);
				if (end <= current) { break; }
			}
		} else {
			// Skip group, all are marked or unallocated.
			current = add_offset(current, unit * bit_group_size);
		}
		alloc->reset();
		alloc++;
		marks++;
	} while (current < end);
	// The black slots are the new allocated set.
	std::swap(bitmap, colors);
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
