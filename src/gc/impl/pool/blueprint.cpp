
/*
ISC License (ISC)

Copyright 2019 Adam Armstrong

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

/**	\file src/gc/impl/pool/blueprint.cpp
 *	\brief Implements gc::blueprint.
 *	\ingroup gc_pool
 */

#include <algorithm>

#include "blueprint.hpp"

#include "../../../unit_test.hpp"
#include "../idaccess.hpp"

#include "bitmap.hpp"
#include "gray_list.hpp"

namespace {

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

std::size_t select_capacity(std::size_t unit) noexcept
{
	using gc::pool;
	using gc::vmem;
	using config::max_pool;
	using config::min_pool;
	std::size_t const max_size{max_pool * vmem::page_size};
	// Check if the minimum object can fit in the maximum number of pages.
	if (unit * min_pool < max_size) {
		// We start with the smallest amount that will completely fill all pages used.
		std::size_t capacity{vmem::page_size / gcd(vmem::page_size, unit)};
		// Get the memory required. This is equal to the least common multiple of unit and page_size.
		std::size_t const size{capacity * unit};
		SYNAFIS_ASSERT(size % vmem::page_size == 0);
		// Ensure we do no exceed config::max_pool pages.
		if (size > max_size) {
			capacity = max_size / unit;
		} else if (size < max_size) {
			// See if we can fit more of the ideal capacity in without exceeding max.
			// Both sides of the division are multiples of vmem::page_size.
			std::size_t part{max_size / size};
			if (2 <= part) { capacity *= part; }
		}
		SYNAFIS_ASSERT(min_pool <= capacity);
		return capacity;
	} else {
		// When config::min_pool is the optimal capacity, this branch is simpler.
		// Capacity must always be at least config::min_pool, even if it exceeds max_pool pages.
		std::size_t capacity{min_pool};
		// Fill up the last occupied page as much as possible.
		std::size_t const remainder{(min_pool * unit) % vmem::page_size};
		if (remainder > unit) { capacity += (vmem::page_size - remainder) / unit; }
		return capacity;
	}
}

}

namespace gc {

blueprint::blueprint(identity const &id, std::size_t u) noexcept
{
	// The unit size of slots must be able to hold a node pointer.
	SYNAFIS_ASSERT(min_unit() <= u);
	unit = std::max(u, min_unit());
	capacity = select_capacity(unit);
	std::size_t const guard{(config::guard_pages ? vmem::page_size : 0)};
	//	Calculate size rounded up to the nearest multiple of vmem::page_size.
	length = capacity * unit;
	std::size_t const remainder{length % vmem::page_size};
	std::size_t const pad{remainder > 0 ? vmem::page_size - (length % vmem::page_size) : 0};
	std::size_t const size{length + pad};
	// Calculate the offset to the end of bit groups.
	maps = bitmap::placement(guard);
	std::size_t const bitmap_length{bitmap::footprint(capacity) * 2};
	std::size_t offset{maps + bitmap_length};
	// Calculate gray stack beginning and end if needed.
	if (idaccess::has_traverser(id)) {
		std::tie(gray, offset) = gray_list::placement(offset, capacity);
	} else {
		gray = 0;
	}
	// Make offset be at the start of a page.
	offset += vmem::page_size - (offset % vmem::page_size);
	header = offset - maps;
	begin = offset + guard;
	total = size + begin + guard;
	SYNAFIS_ASSERT((size - length) / unit == 0);
	SYNAFIS_ASSERT((begin + length) < total);
	SYNAFIS_ASSERT((maps + header) <= begin);
	SYNAFIS_ASSERT(gray == 0 || (maps < gray && gray < (maps + begin)));
	SYNAFIS_ASSERT((unit * capacity) <= length);
}

vmem blueprint::prepare_region() const
{
	vmem mem{total, !config::guard_pages};
	if (mem) {
		// Set up accessible areas if guard pages are used.
		if (config::guard_pages) {
			mem.writable(maps, header);
			mem.writable(begin, length);
		}
		return std::move(mem);
	} else {
		throw std::bad_alloc{};
	}
}

void **blueprint::prepare_gray(identity const &id, vmem const &mem) const noexcept
{
	if (gc::idaccess::has_traverser(id)) {
		return static_cast<void **>(mem[gray]);
	} else {
		return nullptr;
	}
}

}
