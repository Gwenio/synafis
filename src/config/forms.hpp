
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

#ifndef SYNAFIS_CONFIG_FORMS_HPP
#define SYNAFIS_CONFIG_FORMS_HPP
#pragma once

#ifndef SYNAFIS_CONFIG_GENERAL_HPP
#include "general.hpp"
#endif

/**	\file src/config/forms.hpp
 *	\brief Configures spedial forms.
 */

// Set default configurations for those not specified.

/**	\def SYNAFIS_CONFIG_ENV_NOOP
 *	\brief The value for config::remove_env_noop.
 *	\note Define this value in the compiler commandline.
 *	\see config::remove_env_noop
 */
#ifndef SYNAFIS_CONFIG_ENV_NOOP
#define SYNAFIS_CONFIG_ENV_NOOP true
#endif

namespace config {

/**	\var remove_env_noop
 *	\brief Controls whether to remove operation on environments that have no affect.
 *	\details Set to the preprocessor definition SYNAFIS_CONFIG_ENV_NOOP.
 *	\details If optimize is false, then remove_env_noop will also be false.
 */
inline constexpr bool const remove_env_noop = optimize && SYNAFIS_CONFIG_ENV_NOOP;

}

//	Remove preprocessor definitions that are no longer needed.
#undef SYNAFIS_CONFIG_ENV_NOOP

#endif
