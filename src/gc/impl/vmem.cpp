
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

/**	\file src/gc/impl/vmem.cpp
 *	\brief Implements the platform independent portions of gc::vmem.
 *	\ingroup gc_impl
 */

#include "vmem.hpp"

#include <stdexcept>

namespace gc {

void *vmem::operator[](std::size_t offset) const noexcept
{
	SYNAFIS_ASSERT(ptr != nullptr);
	SYNAFIS_ASSERT(offset < len);
	return add_offset(ptr, offset);
}

void *vmem::at(std::size_t offset) const
{
	if (ptr) {
		if (offset < len) {
			return add_offset(ptr, offset);
		} else {
			throw std::out_of_range{"Precondition (offset < len) violated."};
		}
	} else {
		throw std::logic_error{"Precondition (ptr != nullptr) violated."};
	}
}

}
