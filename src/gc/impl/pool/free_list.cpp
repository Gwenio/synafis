
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

/**	\file src/gc/impl/pool/free_list.cpp
 *	\brief Implements gc::free_list.
 *	\ingroup gc_pool
 */

#include "free_list.hpp"

#include "../ptr_math.hpp"

namespace gc {

free_list::free_list(void *begin, std::size_t capacity, std::size_t unit) noexcept :
	head(static_cast<node *>(begin)), space(capacity)
{
	void *const end{add_offset(begin, capacity * unit)};
	node *current{head};
	do {
		// Advance the address.
		begin = add_offset(begin, unit);
		if (begin < end) {
			// Set the next free slot.
			current->next = static_cast<node *>(begin);
			current = current->next;
		} else {
			SYNAFIS_ASSERT(begin == end);
			// There are no more free slots, set nullptr and exit loop.
			current->next = nullptr;
			break;
		}
	} while (true);
}

void *free_list::pop() noexcept
{
	SYNAFIS_ASSERT(!full());
	space--;
	return static_cast<void *>(std::exchange(head, head->next));
}

void free_list::push(void *slot) noexcept
{
	node *temp{static_cast<node *>(slot)};
	temp->next = std::exchange(head, temp);
	space++;
}

}
