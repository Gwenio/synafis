
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

#include "soft_ptr.hpp"
#include "hard_ptr.hpp"

namespace gc {

soft_ptr::soft_ptr(soft_ptr const& other) : soft_ptr() {
	if (other) {
		ptr = copy(other.ptr);
	}
}

soft_ptr::soft_ptr(soft_ptr && other) : soft_ptr() {
	if (other) {
		ptr = update(std::exchange(other.ptr, nullptr));
	}
}

soft_ptr::soft_ptr(hard_ptr const&other) : soft_ptr() {
	if (other) {
		ptr = get_soft(other);
	}
}

soft_ptr::~soft_ptr() {
	if (ptr) {
		free(ptr);
	}
}

soft_ptr &soft_ptr::operator=(std::nullptr_t) noexcept {
	if (ptr) {
		free(ptr);
		ptr = nullptr;
	}
	return *this;
}

soft_ptr &soft_ptr::operator=(soft_ptr const& other) noexcept {
	if (this == std::addressof(other)) {
		refresh();
		return *this;
	} else if (other) {
		if (ptr) {
			free(ptr);
		}
		ptr = copy(other.ptr);
		return *this;
	} else {
		return *this = nullptr;
	}
}

soft_ptr &soft_ptr::operator=(soft_ptr && other) noexcept {
	if (this == std::addressof(other)) {
		refresh();
		return *this;
	} else if (other) {
		if (ptr) {
			free(ptr);
		}
		ptr = update(std::exchange(other.ptr, nullptr));
		return *this;
	} else {
		return *this = nullptr;
	}
}

soft_ptr &soft_ptr::operator=(hard_ptr const& other) noexcept {
	if (other) {
		if (ptr) {
			free(ptr);
		}
		ptr = get_soft(other);
		return *this;
	} else {
		return *this = nullptr;
	}
}

void soft_ptr::refresh() noexcept {
	if (ptr) {
		ptr = update(ptr);
	}
}

}
