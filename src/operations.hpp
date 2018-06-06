
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

#ifndef SYNAFIS_OPERATIONS_HPP
#define SYNAFIS_OPERATIONS_HPP
#pragma once

/**	\file operations.hpp
 *	\brief The primary header for operations.
 */

/**	\dir operations
 *	\brief Contains the headers included by 'operations.hpp'.
 */

#include <type_traits>
#include <tuple>

#include "unit_test.hpp"
#include "datatypes.hpp"

/**	\namespace operation
 *	\brief Contains the definitions of the basic operation for the "virtual machine".
 */

#include "config/operations.hpp"
#include "operations/types.hpp"
#include "operations/peephole.hpp"
#include "operations/block.hpp"
#include "operations/peephole/2.hpp"

/**	\page operation_main Operations
 *	\tableofcontents
 *	\section operation_overview Overview
 *	The choice to implementent the interals of Synafis by layers of template metaprogramming
 *	was decided on because it explores a pair important design goals of Synafis. Those are
 *	compile time code generating run time code and code that modifies other code.
 *
 *	If not to experiment with how these goals can be achieved in an existing language,
 *	then the 'operations' section of the implementation would not exist in the form it does.
 *	"Operations" are a set of classes used to specialize the template functions of datatype::block
 *	to form a set of assembly like instructions.
 *	These instructions are composed into a tuple, which is processed by the function templates
 *	to have the compiler create the needed code for Synafis.
 *
 *	The most notable downside to this design is compile time. Having layers of metatemplates
 *	will certainly cause the compile times to be very long, and they will appear each time
 *	the program is compiled due to the nature of templates. The main way to reduce the time
 *	taken would be to reduce the layers of templates, but the route selected for reason stated
 *	earlier precludes much of a reduction.
 *
 *	The main upside of having the templates go down to to a pseudo-assembly level is that
 *	additional specializations can be added to optimize special cases of code in a modular
 *	manner. The performance of Synafis can be raised latter without directly touching the
 *	implementing code.
 */

#endif
