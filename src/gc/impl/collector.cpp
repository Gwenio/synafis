
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

/**	\file src/gc/impl/collector.cpp
 *	\brief Implements the interface for the garbage collector.
 */

#include "../collector.hpp"
#include "../impl.hpp"

namespace gc {

void initialize() { collector::init(); }

void collect(bool wait) noexcept
{
	if (wait) {
		if (mutex::locked()) {
			collector::wait();
		} else {
			mutex m;
			lock_guard l{m};
			collector::wait();
		}
	} else {
		collector::collect();
	}
}

void set_period(std::chrono::steady_clock::duration value) noexcept
{
	collector::set_period(value);
}

}
