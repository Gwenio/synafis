
/*
ISC License (ISC)

Copyright 2018-2019 Adam Armstrong

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

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

#ifndef SYNAFIS_GC_COLLECTOR_HPP
#define SYNAFIS_GC_COLLECTOR_HPP
#pragma once

/**	\file src/gc/collector.hpp
 *	\brief Defines functions for interacting with the collector.
 *	\todo Add functions for getting collector statistics.
 */

namespace gc {

/**	\fn initialize()
 *	\brief Initializes the collector.
 *	\pre The program has reached main() and has not yet called initialize().
 *	\details Prior to this call, the collector is only setup to handle
 *	\details some operations that are expected to occur before main().
 *	\warning This must be called for the collector to begin operation.
 */
void initialize();

/**	\fn set_period(std::chrono::steady_clock::duration value) noexcept
 *	\brief Sets the time period between unforced GC cycles.
 *	\param value The new period.
 *	\details If the period is set to the special value zero, then collection cycles will only
 *	\details occur when forced with collect() or by running out of memory.
 *	\note The default value for the period is set by config::gc_period.
 */
void set_period(std::chrono::steady_clock::duration value) noexcept;

/**	\fn collect(bool wait) noexcept
 *	\brief Causes the collector to run a collection cycle as soon as possible.
 *	\param wait If true will cause the call to block until a collection cycle has run.
 *	\pre initialize() has been called.
 */
void collect(bool wait) noexcept;

}

#endif
