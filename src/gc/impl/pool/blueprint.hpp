
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

/**	\file src/gc/impl/pool/blueprint.hpp
 *	\brief Defines a type for containing pre-calculated information for creating pools.
 *	\ingroup gc_pool
 */

#include "free_list.hpp"
#include "../../identity.hpp"
#include "../vmem.hpp"

#ifndef SYNAFIS_GC_POOL_BLUEPRINT_HPP
#define SYNAFIS_GC_POOL_BLUEPRINT_HPP
#pragma once

#include <cstddef>

namespace gc {

/**	\struct blueprint
 *	\brief Contains pre-calculated values for constructing a pool.
 *	\ingroup gc_pool
 */
struct blueprint
{
	/**	\fn min_unit()
	 *	\brief The minimum unit size for pool allocations.
	 *	\returns Returns the minimum value for 'unit' in blueprints.
	 */
	static constexpr std::size_t min_unit() { return free_list::node_size(); }

	/**	\var unit
	 *	\brief The size of a slot in pools created with the blueprint.
	 */
	std::size_t unit;

	/**	\var capacity
	 *	\brief The number of slots in pools created with the blueprint.
	 */
	std::size_t capacity;

	/**	\var total
	 *	\brief The total amount of virtual memory to allocate for a pool.
	 */
	std::size_t total;

	/**	\var maps
	 *	\brief The offset from the start of the virtual memory to the bitmaps.
	 *	\invariant maps < total
	 */
	std::size_t maps;

	/**	\var gray
	 *	\brief The offset from the start of the virtual memory to the gray_list.
	 *	\invariant gray == 0 || (gray > maps && gray < total)
	 */
	std::size_t gray;

	/**	\var header
	 *	\brief The length of the part of the virtual memory used for the pool header.
	 *	\invariant maps + header <= begin
	 */
	std::size_t header;

	/**	\var begin
	 *	\brief The offset from the start of the virtual memory to the first slot.
	 *	\invariant begin + length < total
	 */
	std::size_t begin;

	/**	\var length
	 *	\brief The length of the part of the virtual memory used for the slots.
	 *	\invariant unit * capacity <= length
	 */
	std::size_t length;

	/**	\fn blueprint(identity const &id, std::size_t u) noexcept
	 *	\brief Pre-calculates information for creating a pool.
	 *	\param id The type the pool allocates memory for.
	 *	\param u The size of the unit of allocation.
	 *	\pre 'u' must be at least min_unit().
	 */
	blueprint(identity const &id, std::size_t u) noexcept;

	/**	\fn ~blueprint() noexcept
	 *	\brief Default destructor.
	 */
	~blueprint() noexcept = default;

	/**	\prepare_region() const
	 *	\brief Creates a virtual memory region based on the blueprint.
	 *	\returns Returns a gc::vmem that owns a region large enough for pools using the blueprint.
	 *	\throws std::bad_alloc if virtual memory could not be allocated.
	 */
	vmem prepare_region() const;

	/**	\prepare_gray(identity const &id, vmem const &mem) const noexcept
	 *	\brief Gets the start of the gray list for a pool based on the blueprint.
	 *	\param id The gc::identity used to construct the blueprint.
	 *	\param mem A gc::vmem returned by prepare_region().
	 *	\returns Returns a pointer to the start of the gray_list.
	 */
	void **prepare_gray(identity const &id, vmem const &mem) const noexcept;
};

}

#endif
