
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

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

#ifndef SYNAFIS_GC_CALLBACKS_HPP
#define SYNAFIS_GC_CALLBACKS_HPP
#pragma once

/**	\file src/gc/callbacks.hpp
 *	\brief Defines typedefs for callback function pointers.
 */

namespace gc {

/**	\typedef finalize_cb
 *	\brief The callback type for cleaning objects owned by the garbage collector.
 */
typedef void (*finalize_cb)(void *) noexcept;

/**	\typedef enumerate_cb
 *	\brief The callback type for adding a pointer to the reachable set.
 *	\details Provided as a parameter to other callbacks.
 */
typedef void (*enumerate_cb)(void *, void *) noexcept;

/**	\typedef traverse_cb
 *	\brief The callback type for enumerating the pointers to garbage collected objects.
 *	\details The enumerate_cb expects the second parameter as its first.
 */
typedef void (*traverse_cb)(void const*, void *, enumerate_cb) noexcept;

/**	\typedef remap_cb
 *	\brief The callback type for updating pointers to relocated or merged objects.
 *	\details Provided as a parameter to other callbacks.
 */
typedef void *(*remap_cb)(void *, void *) noexcept;

/**	\typedef root_cb
 *	\brief The callback type for remapping the pointers in root objects.
 *	\details The remap_cb expects the second parameter as its first.
 */
typedef void (*root_cb)(void *, void *, remap_cb) noexcept;

/**	\typedef relocate_cb
 *	\brief The callback type for relocating objects.
 *	\details The remap_cb expects the third parameter as its first.
 */
typedef void (*relocate_cb)(void *, void *, void *, remap_cb) noexcept;

/**	\typedef equality_cb
 *	\brief The callback type for checking if two objects will always be equal.
 */
typedef bool (*equality_cb)(void const*, void const*) noexcept;

}

#endif
