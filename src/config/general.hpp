
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

#ifndef SYNAFIS_CONFIG_GENERAL_HPP
#define SYNAFIS_CONFIG_GENERAL_HPP
#pragma once

/**	\dir config
 *	\brief Contains the configuration header files.
 */

/**	\file src/config/general.hpp
 *	\brief General configurations useful to all parts of the program.
 */

// Set default configurations for those not specified.

#ifdef NDEBUG
#define SYNAFIS_CONFIG_DEBUG false
#else
#define SYNAFIS_CONFIG_DEBUG true
#endif

/**	\namespace config
 *	\brief Stores configuration constants for implementation details.
 */
namespace config {

/**	\var debug
 *	\brief Controls the use of debugging helpers.
 *	\details Set to the preprocessor definition SYNAFIS_CONFIG_DEBUG.
 *	\details Indirectly set by having NDEBUG or DEBUG defined.
 */
inline constexpr bool const debug = SYNAFIS_CONFIG_DEBUG;

}

#endif
