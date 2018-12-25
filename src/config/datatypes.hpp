
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

#ifndef SYNAFIS_CONFIG_DATATYPES_HPP
#define SYNAFIS_CONFIG_DATATYPES_HPP
#pragma once

#ifndef SYNAFIS_CONFIG_GENERAL_HPP
#include "general.hpp"
#endif

/**	\file src/config/datatypes.hpp
 *	\brief Configures implementation details of datatypes.
 */

// Set default configurations for those not specified.

/**	\def SYNAFIS_CONFIG_MUTABLE_ENV
 *	\brief The value for config::mutable_env.
 *	\note Define this value in the compiler commandline.
 *	\see config::mutable_env
 */
#ifndef SYNAFIS_CONFIG_MUTABLE_ENV
#define SYNAFIS_CONFIG_MUTABLE_ENV false
#endif

/**	\def SYNAFIS_CONFIG_KEEP_CALL
 *	\brief The value for config::keep_call.
 *	\note Define this value in the compiler commandline.
 *	\see config::keep_call
 */
#ifndef SYNAFIS_CONFIG_KEEP_CALL
#define SYNAFIS_CONFIG_KEEP_CALL (!mutable_env)
#endif

namespace config {

/**	\var mutable_env
 *	\brief Controls the method through which environment key-value bindings are added.
 *	\details When true bindings mutates the environemnt; otherwise, a new environment
 *	\details is created as a child of the old environment that contains the binding
 *	\details to become the new current environemnt.
 */
constexpr bool const mutable_env = SYNAFIS_CONFIG_MUTABLE_ENV;

/**	\var keep_call
 *	\brief If true then called closures will remain on top of the stack after exec_call.
 *	\details This is a work around to make it possible to have recursive closures
 *	\details when environments are immutable.
 *	\see operation::exec_call
 */
constexpr bool const keep_call = SYNAFIS_CONFIG_KEEP_CALL;

static_assert(mutable_env || keep_call,
	"Environments must be mutable or we must keep closures on the stack after calling them.");

}

#endif
