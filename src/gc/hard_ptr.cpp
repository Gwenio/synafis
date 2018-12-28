
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

#include "hard_ptr.hpp"

namespace gc {

hard_ptr::hard_ptr(soft_ptr const &other) : hard_ptr()
{
	if (other) {
		std::tie(ptr, type) = get_hard(other);
		SYNAFIS_ASSERT(ptr == nullptr || type != nullptr);
	}
}

hard_ptr &hard_ptr::operator=(soft_ptr const &other)
{
	if (other) {
		std::tie(ptr, type) = get_hard(other);
		return *this;
	} else {
		return *this = nullptr;
	}
}

hard_ptr &hard_ptr::operator=(hard_ptr const &other) noexcept
{
	if (this != std::addressof(other)) {
		ptr = other.ptr;
		type = other.type;
	}
	return *this;
}

hard_ptr &hard_ptr::operator=(hard_ptr &&other) noexcept
{
	if (this != std::addressof(other)) {
		ptr = std::exchange(other.ptr, nullptr);
		type = std::exchange(other.type, nullptr);
	}
	return *this;
}

bool hard_ptr::operator==(hard_ptr const &other) const noexcept
{
	return ptr == other.ptr ||
		   (type == other.type && type != nullptr && type->equal(cptr(ptr), cptr(other.ptr)));
}

bool hard_ptr::operator!=(hard_ptr const &other) const noexcept
{
	// If ptr != other.ptr && type != other.type, then type != nullptr
	return ptr != other.ptr && (type != other.type || type->equal(cptr(ptr), cptr(other.ptr)));
}

identity const &hard_ptr::get_ident() const
{
	if (type) {
		return *type;
	} else {
		throw std::logic_error{"type points to null."};
	}
}

}
