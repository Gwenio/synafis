
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

/**	\file src/gc/impl/hard_ptr.cpp
 *	\brief Implements portions of gc::hard_ptr that need access to the GC's internals.
 */

#include "../hard_ptr.hpp"
#include "../identity.hpp"
#include "soft_ptr_data.hpp"
#include "core.hpp"

namespace gc {

std::tuple<void *, identity const *> hard_ptr::get_hard(soft_ptr const &other)
{
	using rtype = std::tuple<void *, identity const *>;
	if (other.ptr && other.ptr->get()) {
		return rtype{other.ptr->get(), identity::fetch(other.ptr->get(), std::nothrow)};
	} else {
		return rtype{nullptr, nullptr};
	}
}

void *hard_ptr::base_ptr(void *source) noexcept
{
	return source ? collector::base_ptr(source) : static_cast<void *>(nullptr);
}

}
