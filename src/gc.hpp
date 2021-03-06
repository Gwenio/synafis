
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

/**	\defgroup gc_group GC
 *	\brief Contains garbage collector components.
 */

/**	\defgroup gc_interface Interface
 *	\brief Group containing implementation details of the garbage collector.
 *	\ingroup gc_group
 */

/**	\defgroup gc_impl Implementation
 *	\brief Group containing implementation details of the garbage collector.
 *	\ingroup gc_group
 */

/**	\file src/gc.hpp
 *	\brief The primary header for garbage collector code.
 *	\ingroup gc_interface
 */

#ifndef SYNAFIS_GC_HPP
#define SYNAFIS_GC_HPP
#pragma once

/**	\dir src/gc
 *	\brief Contains the headers included by 'gc.hpp'.
 *	\ingroup gc_interface
 */

/**	\dir src/gc/impl
 *	\brief Contains implementation details of the garbage collector.
 *	\ingroup gc_impl
 */

/**	\namespace gc
 *	\brief Contains garbage collector utilities.
 *	\ingroup gc_group
 */

#include "config/gc.hpp"
#include "gc/callbacks.hpp"
#include "gc/traits.hpp"
#include "gc/identity.hpp"
#include "gc/lock.hpp"
#include "gc/soft_ptr.hpp"
#include "gc/hard_ptr.hpp"
#include "gc/root.hpp"
#include "gc/collector.hpp"

/**	\page gc_main Garbage Collector
 *	\tableofcontents
 *	\section gc_overview Overview
 *	The garbage collector is split into an interface visible to the program
 *	and the implementation that is to be hidden in a separate compilation unit.
 *
 *	The provided interface is for a tracing collector with exact knowledge of
 *	points to the objects it manages. The collector does not scan the stack,
 *	it only traces from registered root objects.
 *
 *	A set of traits are defined to determine how the collector should interact
 *	with objects of a given type, and the traits are collected into an identity
 *	which represents the type for the collector.
 *
 *	The pointer types for the collector erase the type of the object they point
 *	to except for the identity of the type. These pointers are broken down
 *	into hard pointers that keep the object alive and soft pointer which
 *	do not. Soft pointers instead allow a hard pointer to be retrieve if
 *	the object still exists.
 *	\ingroup gc_group
 */

#endif
