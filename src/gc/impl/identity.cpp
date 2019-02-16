
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

/**	\file src/gc/impl/identity.cpp
 *	\brief Implements portions of gc::identity that need access to the GC's internals.
 *	\ingroup gc_impl
 */

#include "../identity.hpp"
#include "core.hpp"
#include "allocator.hpp"

namespace gc {

identity::~identity()
{
	if (alloc) { collector::erase_alloc(*dynamic_cast<collector::iallocator *>(alloc)); }
}

identity::iallocator *identity::select_alloc(
	identity const &id, std::size_t unit, traits::flag_type flags)
{
	auto *ptr = collector::insert_alloc(collector::alloc_ptr{
		dynamic_cast<collector::iallocator *>(new allocator(id, unit, flags))});
	return dynamic_cast<iallocator *>(ptr);
}

identity const *identity::fetch_impl(void *obj) noexcept { return collector::get_type(obj); }

}
