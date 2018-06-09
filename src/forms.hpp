
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

#ifndef SYNAFIS_FORMS_HPP
#define SYNAFIS_FORMS_HPP
#pragma once

/**	\file forms.hpp
 *	\brief The primary header for operations.
 */

/**	\dir forms
 *	\brief Contains the headers included by 'forms.hpp'.
 */

#include <type_traits>
#include <tuple>

#include "unit_test.hpp"
#include "operations.hpp"

/**	\namespace form
 *	\brief Contains the definitions of the basic operation for the "virtual machine".
 */

#include "config/forms.hpp"
#include "forms/common.hpp"
#include "forms/basic.hpp"
#include "forms/control.hpp"

/**	\page form_main Special Forms
 *	\tableofcontents
 *	\section form_overview Overview
 *	Special forms are template classes that chain operations together into
 *	useful sequences. They provide a member typedef 'impl' that takes a tuple
 *	of following operations for the block as a template parameter.
 *
 *	Forms may have their implementation vary based on whether they are in a
 *	tail context or not. A tail context means the next step is to return.
 *	Or in terms of the available operations, the next operation is exec_call.
 *	The goal is to implement proper tail recursion, as in Scheme.
 *
 *	Forms are not the final abstraction layer for the implementation of Synafis.
 *	Above them are 'macro' templates that reduce common boiler plate code.
 *	So forms will have a corresponding macro that expands on them to make their
 *	usage easier, but they are split up to keep things from getting to complex.
 */

#endif
