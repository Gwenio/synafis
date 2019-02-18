
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

/**	\file src/gc/impl/pool/arena.cpp
 *	\brief Implements gc::arena.
 *	\ingroup gc_pool
 */

#include "arena.hpp"

namespace gc {

void *arena::operator[](std::size_t offset) noexcept
{
	SYNAFIS_ASSERT(offset < max())
	return static_cast<std::byte *>(front) + (offset * unit);
}

std::size_t arena::get_slot(void *slot) const noexcept
{
	SYNAFIS_ASSERT(from(slot));
	std::ptrdiff_t const x{static_cast<std::byte *>(slot) - static_cast<std::byte *>(front)};
	return x / unit;
}

}
