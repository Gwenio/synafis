
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

#if !defined(SYNAFIS_OPERATIONS_HPP)
#include "../operations.hpp"
#elif !defined(SYNAFIS_CONFIG_OPERATIONS_HPP)
#define SYNAFIS_CONFIG_OPERATIONS_HPP
#pragma once

#ifndef SYNAFIS_CONFIG_GENERAL_HPP
#include "general.hpp"
#endif

/**	\file src/config/operations.hpp
 *	\brief Configures implementation details for operations.
 */

// Set default configurations for those not specified.

#ifndef SYNAFIS_CONFIG_OPTIMIZE
#define SYNAFIS_CONFIG_OPTIMIZE (!config::debug)
#endif

#ifndef SYNAFIS_CONFIG_PEEPHOLE
#define SYNAFIS_CONFIG_PEEPHOLE true
#endif

namespace config {

/**	\var optimize
 *	\brief A general control for using optimized routines for datatype::block::impl().
 *	\details Set to the preprocessor definition SYNAFIS_CONFIG_OPTIMIZE.
 *	\warning To be useful the code units that add optimizations must be respecting this configuration.
 *	\note Defaults to the opposite of debug.
 */
inline constexpr bool const optimize = SYNAFIS_CONFIG_OPTIMIZE;

/**	\var peephole
 *	\brief Controls the use of peephole optimizations.
 *	\details Set to the preprocessor definition SYNAFIS_CONFIG_PEEPHOLE.
 *	\details If optimize is false, then peephole will also be false.
 *	\warning To be useful the code units that add peepholes must be respecting this configuration.
 */
inline constexpr bool const peephole = optimize && SYNAFIS_CONFIG_PEEPHOLE;

}

#endif
