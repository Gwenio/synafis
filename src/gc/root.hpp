
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

#ifndef SYNAFIS_GC_HPP
#include "../gc.hpp"
#endif

#ifndef SYNAFIS_GC_ROOT_HPP
#define SYNAFIS_GC_ROOT_HPP
#pragma once

/**	\file src/gc/root.hpp
 *	\brief Defines tools for (un)registering root objects with the collector.
 */

namespace gc {

/**	\class root
 *	\brief Class for managing root objects.
 *	\details Objects of the root class can be used to automatically manage
 *	\details registration and unregistering roots.
 *	\details
 *	\details For types that are always roots, static members are provides
 *	\details so they can manage themselves.
 */
class root
{
	//!	\cond friends
	friend root;
	friend unit_test::tester<root>;
	//!	\endcond
private:
	/**	\fn register_impl(void *obj, traverse_cb tcb, root_cb rcb)
	 *	\brief Registers an object as a root with the collector.
	 *	\param obj The object to register.
	 *	\param tcb The callback for traversing pointers in the object.
	 *	\param rcb The callback for updating pointers in the object.
	 *	\pre Must be called in the scope of a gc::scope_lock.
	 *	\pre obj != nullptr
	 *	\pre tcb != nullptr
	 *	\pre rcb != nullptr
	 *	\pre The the object is not yet registered.
	 *	\post The object is now registered as a root object.
	 *	\details Implemented in the collector.
	 */
	static void register_impl(void *obj, traverse_cb tcb, root_cb rcb);

	/**	\fn unregister_impl(void *obj) noexcept
	 *	\brief Unregisters a root object.
	 *	\param obj The object to unregister.
	 *	\pre obj != nullptr
	 *	\pre The the object must be registered as a root via register_root.
	 *	\post The object is no longer registered as a root.
	 *	\details Implemented in the collector.
	 */
	static void unregister_impl(void *obj) noexcept;

	/**	\fn root_impl(void *obj, void *data, remap_cb cb) noexcept
	 *	\brief Implementation for root_cb to update pointers in root objects.
	 *	\tparam T The type of root object.
	 *	\param obj The object having its pointers updated.
	 *	\param data Additional argument to pass to cb.
	 *	\param cb The remap callback for type T.
	 *	\pre obj != nullptr && cb != nullptr
	 *	\details Wraps remap<T>, providing the correct parameters for registering.
	 *	\warning Should only be called by the collector.
	 */
	template<typename T>
	void root_impl(void *obj, void *data, remap_cb cb) noexcept
	{
		static_cast<T *>(obj)->remap(data, cb);
	}

	/**	\fn get_pointer(T &obj) noexcept
	 *	\brief Gets the pointer to an object.
	 *	\tparam T The type of root object.
	 *	\param obj The object to get a pointer to.
	 *	\returns Returns a pointer to obj.
	 */
	template<typename T>
	constexpr static void *get_pointer(T &obj) noexcept
	{
		return static_cast<void *>(const_cast<std::remove_cv_t<T> *>(std::addressof(obj)));
	}

	/**	\var obj
	 *	\brief Pointer to the root object.
	 */
	void *obj;

public:
	/**	\fn root()
	 *	\brief No root is owned.
	 */
	constexpr root() noexcept : obj(nullptr) {}

	/**	\fn root(root const &)
	 *	\brief Deleted.
	 */
	root(root const &) = delete;

	/**	\fn root(root &&other)
	 *	\brief Moves ownership of the root from other to this.
	 *	\param other The root to take ownership from.
	 */
	root(root &&other) noexcept : obj(std::exchange(other.obj, nullptr)) {}

	/**	\fn root(T &ref)
	 *	\brief Creates a new root reference owned by this.
	 *	\tparam T The type of root object.
	 *	\param ref A reference to register.
	 *	\pre Must be called in the scope of a gc::scope_lock.
	 *	\pre The the object is not yet registered.
	 */
	template<typename T>
	root(T &ref) : obj(get_pointer(ref))
	{
		register_root(ref);
	}

	/**	\fn ~root()
	 *	\brief Unregisters the root object if we have one.
	 */
	~root()
	{
		if (obj) { unregister_impl(obj); }
	}

	/**	\fn root &operator=(root const &)
	 *	\brief Deleted.
	 */
	root &operator=(root const &) = delete;

	/**	\fn root &operator=(root &&other)
	 *	\brief Moves ownership of the root from other to this.
	 *	\param other The root to take ownership from.
	 *	\returns Returns *this.
	 *	\post 'other' no longer owns a root object.
	 *	\details Unregisters the previous owned root if there was one.
	 */
	root &operator=(root &&other) noexcept
	{
		if (this != std::addressof(other)) {
			if (obj) { unregister_impl(obj); }
			obj = std::exchange(other.obj, nullptr);
		}
		return *this;
	}

	/**	\fn register_(T &obj)
	 *	\brief Registers an object as a root with the collector.
	 *	\tparam T The type of obj.
	 *	\param obj The object to register.
	 *	\pre Must be called in the scope of a gc::scope_lock.
	 *	\pre The the object is not yet registered.
	 *	\post The object is now registered as a root object.
	 *	\details Provides the correct callbacks for type T during registration.
	 */
	template<typename T>
	static void register_(T &obj)
	{
		static_assert(traits::pointers<T>,
			"The type of a root object must be specified as gc::traits::pointers<T> == true.")
			register_impl(get_pointer(obj), traits::traverser<T>, (&root_impl<T>));
	}

	/**	\fn unregister(T &obj)
	 *	\brief Unregisters a root object.
	 *	\tparam T The type of obj.
	 *	\param obj The object to unregister.
	 *	\pre The the object must be registered as a root via register_root.
	 *	\post The object is no longer registered as a root.
	 */
	template<typename T>
	static void unregister(T &obj)
	{
		unregister_impl(get_pointer(obj));
	}
};

}

#endif
