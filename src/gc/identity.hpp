
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

#if !defined(SYNAFIS_GC_HPP)
#include "../gc.hpp"
#elif !defined(SYNAFIS_GC_IDENTITY_HPP)
#define SYNAFIS_GC_IDENTITY_HPP
#pragma once

/**	\file gc/identity.hpp
 *	\brief Defines tools for identifying types to the garbage collector.
 */

namespace gc {

/**	\class identity
 *	\brief Contains callbacks used for the type the identity stands for.
 *	\details Allows types to be interacted with by the collector in the
 *	\details way it needs to without knowing their type.
 *	\invariant acb != nullptr
 *	\note This could be done with a base class with virtual methods,
 *	\note but the collector has nothing to do with the class heirachy.
 *	\note
 *	\note In particular, having all types used with the collector
 *	\note inherit from a common base class means that if multiple
 *	\note inheritance is used it may lead to messy diamond inheritance.
 *	\note
 *	\note Also, separating the identity from the class heirachy means
 *	\note a type can have additional identities for the presence of
 *	\note const and/or volatile as qualifiers.
 */
class identity {
	friend unit_test::tester<identity>;
public:
	/**	\class access
	 *	\brief Type to access restricted parts of the identity class.
	 *	\note Should be defined where only the garbage collector has access
	 *	\note to the members of this class.
	 */
	class access;

	// So the garbage collector internals can get more access.
	friend access;

	/**	\typedef allocate_cb
	 *	\brief The callback type for allocating a garbage collector owned object.
	 */
	typedef void *(*allocate_cb)(identity const&, void *) noexcept;
private:
	/**	\var allocator
	 *	\brief Data provided for passing to the allocate callback (acb).
	 *	\see select_alloc
	 */
	void *allocator;

	/**	\var acb
	 *	\brief The callback used to allocate objects from the collector.
	 *	\see select_alloc
	 */
	allocate_cb acb;

	/**	\var fcb
	 *	\brief The callback used to clean up an object.
	 */
	finalize_cb fcb;

	/**	\var tcb
	 *	\brief The callback used to enumerate pointers reachable from an object.
	 */
	traverse_cb tcb;

	/**	\var rcb
	 *	\brief The callback used to relocate objects and remap their pointers.
	 */
	relocate_cb rcb;

	/**	\var ecb
	 *	\brief The callback used check if two objects will always be equal.
	 */
	equality_cb ecb;

	/**	\fn unit_size() noexcept
	 *	\brief Gets the amount of memory to allocate to objects of a type.
	 *	\tparam T The type to get the allocation unit for.
	 *	\returns Returns the size of the allocation unit for objects of type T.
	 *	\details The return value is to be at least sizeof(T) and a multiple of alignof(T).
	 */
	template<typename T>
	static constexpr std::size_t unit_size() noexcept {
		return (sizeof(T) % alignof(T) == 0) ? sizeof(T) :
			((sizeof(T) / alignof(T)) + 1) * alignof(T);
	}

	/**	\fn select_alloc(identity const&id, std::size_t unit, traits::flag_type flags)
	 *	\brief Gets the allocation callback and the allocator data.
	 *	\param id The identity of the type to get an allocator for.
	 *	\param unit The return value of unit_size for the type to select an allocator for.
	 *	\param flags The flags from traits::get_flags.
	 *	\returns Returns a tuple with the allocator data and callback.
	 *	\returns The data maybe nullptr, but the callback must be valid.
	 *	\pre Is to only be called once per identity object.
	 *	\see traits::get_flags
	 */
	static std::tuple<void *, allocate_cb>
	select_alloc(identity const&id, std::size_t unit, traits::flag_type flags);

	/**	\fn fetch_impl(void *obj) noexcept
	 *	\brief Gets the identity of an object.
	 *	\param obj The object to get the identity for.
	 *	\returns Returns a pointer to the identity of the object or nullptr if
	 *	\returns the object was not allocated by the collector.
	 *	\pre obj != nullptr
	 */
	static identity const* fetch_impl(void *obj) noexcept;

	/**	\fn identity(finalize_cb f, traverse_cb t, relocate_cb r, equality_cb e) noexcept
	 *	\brief Sets up the parts that can be done as a constexpr.
	 *	\param f The finalizer callback.
	 *	\param t The traversal callback.
	 *	\param r The relocation and remapping callback.
	 *	\param e The equality check callback.
	 */
	constexpr identity(finalize_cb f, traverse_cb t, relocate_cb r, equality_cb e) noexcept :
		allocator(nullptr), acb(nullptr), fcb(f), tcb(t), rcb(r), ecb(e) {}

	/**	\fn finalize(void *obj) const noexcept
	 *	\brief Calls the finalizer callback if it is present.
	 *	\param obj The address of the object to clean up.
	 *	\pre The object is unreachable outside the collector.
	 *	\pre obj != nullptr
	 *	\details Private so it is not accessible to the program.
	 *	\details The collector should access this function via detail::idaccess.
	 */
	void finalize(void *obj) const noexcept {
		if (fcb) {
			SYNAFIS_ASSERT(obj != nullptr);
			(*fcb)(obj);
		}
	}

	/**	\fn traverse(void const* obj, void *data, enumerate_cb cb) const noexcept
	 *	\brief Calls the traversal callback if it is present.
	 *	\param obj The address of the object to traverse.
	 *	\param data Optional data for the callback, may be nullptr.
	 *	\param cb The callback for passing pointers in obj to.
	 *	\pre obj != nullptr && cb != nullptr
	 *	\details The callback 'cb' expects data as the first parameter and the pointer second.
	 *	\details
	 *	\details Private so it is not accessible to the program.
	 *	\details The collector should access this function via detail::idaccess.
	 */

	void traverse(void const* obj, void *data, enumerate_cb cb) const noexcept {
		if (tcb) {
			SYNAFIS_ASSERT(obj != nullptr);
			SYNAFIS_ASSERT(cb != nullptr);
			(*tcb)(obj, data, cb);
		}
	}
	/**	\fn relocate(void *orig, void *dest, void *data, remap_cb cb) const noexcept
	 *	\brief Moves an object and remaps its pointers.
	 *	\param orig The address to move the object from.
	 *	\param dest The address to move the object to.
	 *	\param data Optional data for the callback, may be nullptr.
	 *	\param cb The callback for passing pointers in obj to which then provides the new address for.
	 *	\pre dest != nullptr && cb != nullptr
	 *	\details The parameters 'orig' and 'dest' will be equal if the object does not move.
	 *	\details
	 *	\details The callback 'cb' expects data as the first parameter and the pointer second.
	 *	\details
	 *	\details Private so it is not accessible to the program.
	 *	\details The collector should access this function via detail::idaccess.
	 */
	void relocate(void *orig, void *dest, void *data, remap_cb cb) const noexcept {
		if (rcb) {
			SYNAFIS_ASSERT(dest != nullptr);
			SYNAFIS_ASSERT(cb != nullptr);
			(*rcb)(orig, dest, data, cb);
		}
	}
public:
	/**	\fn identity(T *)
	 *	\brief Constructs the identity for a type.
	 *	\tparam T The type to construct an identity for.
	 *	\post acb != nullptr
	 *	\post !traits::pointers<T> || (tcb && rcb)
	 *	\post std::is_trivially_destructible_v<T> || fcb != nullptr
	 *	\post !traits::movable<T> || std::is_trivially_copyable_v<T> || rcb != nullptr
	 *	\details Sets the allocator and preforms sanity checks if
	 *	\details assertion are enabled.
	 */
	template<typename T>
	identity(T *) : identity(traits::finalizer<T>, traits::traverser<T>,
		traits::relocator<T>, traits::equalizer<T>) {
			std::tie(allocator, acb) =
				select_alloc(*this, unit_size<T>(), traits::get_flags<T>());
			SYNAFIS_ASSERT(acb != nullptr);
			SYNAFIS_ASSERT(!traits::pointers<T> || (tcb && rcb));
			SYNAFIS_ASSERT(std::is_trivially_destructible_v<T> || fcb != nullptr);
			SYNAFIS_ASSERT(!traits::movable<T> || std::is_trivially_copyable_v<T> || rcb != nullptr);
		}

	/**	\fn ~identity()
	 *	\brief Cleans up the allocator if needed.
	 *	\note This is to be implemented in the garbage collector implementation.
	 */
	~identity();

	/**	\fn allocate() const noexcept
	 *	\brief Calls the finalizer callback if it is present.
	 *	\param obj The address of the object to clean up.
	 *	\returns Returns a non-null point on success.
	 *	\returns If nullptr is returned, then it means allocation failed and
	 *	\returns the collector could not free enough memory at this time.
	 *	\pre The collector lock must be held by the calling thread.
	 *	\details Private so it is not accessible to the program.
	 *	\details The collector should access this function via detail::idaccess.
	 */
	void *allocate() const noexcept {
		SYNAFIS_ASSERT(acb != nullptr);
		return (*acb)(*this, allocator);
	}

	/**	\fn equal(void const* lhs, void const* rhs) const noexcept
	 *	\brief Checks that two objects will always be equal.
	 *	\param lhs The first object.
	 *	\param rhs The second object.
	 *	\pre lhs != rhs && lhs != nullptr && rhs != nullptr
	 *	\returns Returns true if the objects will never not be equal.
	 *	\details Public so that the program can use it to make sure objects
	 *	\details that should be considered as the same object will be treated as such.
	 */
	bool equal(void const* lhs, void const* rhs) const noexcept {
		SYNAFIS_ASSERT(lhs != rhs);
		if (ecb) {
			SYNAFIS_ASSERT(lhs != nullptr);
			SYNAFIS_ASSERT(rhs != nullptr);
			return (*ecb)(lhs, rhs);
		} else {
			// Objects are not equal if their equality cannot be checked.
			return false;
		}
	}

	/**	\fn fetch(T *obj)
	 *	\brief Gets the identity of an object.
	 *	\tparam T The apperent type of obj.
	 *	\param obj The object to get the identity for.
	 *	\returns Returns a reference to the identity of the object.
	 *	\throws Throws std::runtime_error if the object was not allocated by the collector.
	 *	\pre obj != nullptr
	 *	\pre obj points to an area of memory allocated the the collector.
	 */
	template<typename T>
	static identity const& fetch(T *obj) {
		SYNAFIS_ASSERT(obj != nullptr);
		identity const* temp{fetch_impl(static_cast<void *>(const_cast<std::remove_cv_t<T> *>(obj)))};
		if (temp) {
			return *temp;
		} else {
			throw std::runtime_error("Object was not allocated by the garbage collector.");
		}
	}

	/**	\fn fetch(T *obj, std::nothrow_t) noexcept
	 *	\brief Gets the identity of an object.
	 *	\tparam T The apperent type of obj.
	 *	\param obj The object to get the identity for.
	 *	\returns Returns a pointer to the identity of the object or nullptr if
	 *	\returns the object was not allocated by the collector.
	 *	\pre obj != nullptr
	 */
	template<typename T>
	static identity const* fetch(T *obj, std::nothrow_t) noexcept {
		SYNAFIS_ASSERT(obj != nullptr);
		return fetch_impl(static_cast<void *>(const_cast<std::remove_cv_t<T> *>(obj)));
	}
};

/**	\fn get_id() noexcept
 *	\brief Function to the identity for a type T.
 *	\tparam The type to get the identity for.
 *	\returns Returns a reference to the identity for the type.
 *	\note A type may have separate identities for the presence of const or volatile
 *	\note as qualifies.
 *	\todo Work out the best way for providing the identities for types.
 */
template<typename T>
identity const& get_id() noexcept;

}

#endif
