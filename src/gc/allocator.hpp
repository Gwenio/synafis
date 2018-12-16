
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

#ifndef SYNAFIS_GC_ALLOCATOR_HPP
#define SYNAFIS_GC_ALLOCATOR_HPP
#pragma once

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

#ifndef SYNAFIS_GC_POOL_HPP
#include "pool.hpp"
#endif

/**	\file gc/allocator.hpp
 *	\brief Defines the type that manages the pools for a type of object.
 */

namespace gc {

/**	\class allocator
 *	\brief Type to manage pools for a type of object.
 */
class allocator : public identity::iallocator {
	friend unit_test::tester<allocator>;

	allocator() = delete;

	allocator(allocator const&) = delete;

	allocator(allocator &&) = delete;
private:
public:
	allocator(identity const&id, std::size_t unit, traits::flag_type flags) :
		iallocator() {}

	virtual ~allocator() {}

	virtual void *allocate() override {
		return nullptr;
	}

	virtual void *allocate(std::nothrow_t) noexcept override {
		return nullptr;
	}
};

}

#endif
