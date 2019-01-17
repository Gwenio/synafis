
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

/**	\file test/gc/lock.cpp
 *	\brief Defines test cases for the ::gc::basic_mutex and ::gc::debug_mutex.
 */

#include "gc.hpp"
#include "../../src/gc/lock.hpp"
#include "collector.hpp"

namespace {

using unit_test::collector;
using gc::basic_mutex;
using gc::debug_mutex;
using gc_tester = unit_test::tester<gc::collector>;

template<typename Mutex>
void locking(collector &)
{
	Mutex mtx;
	if (Mutex::locked()) {
		SYNAFIS_FAILURE("The collector lock was locked before the test.");
		return;
	} else if (gc_tester::has_readers()) {
		SYNAFIS_FAILURE("The collector should have no readers when this test starts.");
		return;
	} else {
		std::lock_guard<Mutex> l{mtx};
		SYNAFIS_ASSERT(Mutex::locked());
		SYNAFIS_ASSERT(gc_tester::has_readers());
	}
	SYNAFIS_ASSERT(!Mutex::locked());
	SYNAFIS_ASSERT(gc_tester::no_readers());
}

using c = unit_test::case_type;
using unit_test::pass;
using unit_test::fail;
using unit_test::skip;

inline unit_test::suite &s{unit_test::gc::lock_suite};

static c debug_locking{"debug locking", s, pass, locking<debug_mutex>};

static c basic_locking{"basic_locking", s, pass, locking<basic_mutex>};

}
