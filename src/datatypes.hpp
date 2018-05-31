
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

#ifndef SYNAFIS_DATATYPES_HPP
#define SYNAFIS_DATATYPES_HPP
#pragma once

/**	\file datatypes.hpp
 *	\brief The primary header for datatypes.
 */

/**	\dir datatypes
 *	\brief Contains the headers included by 'datatypes.hpp'.
 */

#include <type_traits>
#include <utility>
#include <variant>

#include "unit_test.hpp"
#include "gc.hpp"

/**	\namespace datatype
 *	\brief Contains the basic datatypes for Synafis.
 */

#include "datatypes/config.hpp"
#include "datatypes/literals.hpp"
#include "datatypes/block.hpp"
#include "datatypes/value_type.hpp"
#include "datatypes/environment_type.hpp"
#include "datatypes/state_type.hpp"

#endif
