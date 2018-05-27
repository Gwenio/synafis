
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
#elif !defined(SYNAFIS_GC_HARD_PTR_HPP)
#define SYNAFIS_GC_HARD_PTR_HPP
#pragma once

/**	\file gc/hard_ptr.hpp
 *	\brief Defines the type for hard garbage collector pointers.
 */

namespace gc {

class soft_ptr;

/**	\class hard_ptr
 *	\brief Type for managing hard pointers.
 *	\invariant If ptr == nullptr then type must also be nullptr.
 *	\invariant If ptr != nullptr then type must not be nullptr.
 */
class hard_ptr {
	friend soft_ptr;
	friend hard_ptr;
	friend unit_test::tester<hard_ptr>;
private:
	/**	\var ptr
	 *	\brief Pointer to object.
	 */
	void *ptr;
	/**	\var type
	 *	\brief The type of the object ptr points to.
	 *	\note The identity of an object can be found even if not stored.
	 *	\note This is just to save the look up.
	 */
	identity const* type;

	/**	\fn get_hard(soft_ptr const&other)
	 *	\brief Gets the hard pointer from a soft pointer.
	 *	\param other The soft_ptr to get the hard pointer for.
	 *	\returns Returns a tuple containing the values for ptr and type.
	 */
	static std::tuple<void *, identity const*> get_hard(soft_ptr const&other);

	/**	\fn base_ptr(void *source) noexcept
	 *	\brief Gets the base address of the object pointed to by source.
	 *	\param source The pointer to get the base address for.
	 *	\returns Returns the base address if source points to an area allocated by
	 *	\return the collector; otherwise, nullptr is returned.
	 *	\details The "base pointer" is the address originally returned by identity::allocate.
	 */
	static void *base_ptr(void *source) noexcept;

	/**	\fn void *mptr(T *source) noexcept
	 *	\brief Casts a pointer to a void pointer.
	 *	\tparam T The type to cast from.
	 *	\param source The pointer to cast.
	 *	\returns source cast to void*.
	 */
	template<typename T>
	constexpr static void *mptr(T *source) noexcept {
		return static_cast<void *>(const_cast<std::remove_cv_t<T> *>(source));
	}

	/**	\fn void *cptr(T *source) noexcept
	 *	\brief Casts a pointer to a const void pointer.
	 *	\tparam T The type to cast from.
	 *	\param source The pointer to cast.
	 *	\returns source cast to void const*.
	 */
	template<typename T>
	constexpr static void const* cptr(T *source) noexcept {
		return static_cast<void const*>(const_cast<std::add_const_t<std::remove_cv_t<T>> *>(source));
	}
public:
	/**	\fn hard_ptr() noexcept
	 *	\brief Initialize with a null pointer and no type.
	 */
	constexpr hard_ptr() noexcept : ptr(nullptr), type(nullptr) {}

	/**	\fn hard_ptr(std::nullptr_t) noexcept
	 *	\brief Initialize with a null pointer and no type.
	 */
	constexpr hard_ptr(std::nullptr_t) noexcept : hard_ptr() {}

	/**	\fn hard_ptr(T *obj)
	 *	\brief Tries to initialize the hard_ptr with obj.
	 *	\tparam The type pointed to by obj.
	 *	\param obj The object to try making a hard_ptr for.
	 *	\throws Throws std::runtime_error if unable to get an identity for obj.
	 *	\post ptr != nullptr && type != nullptr
	 */
	template<typename T>
	hard_ptr(T *obj) : ptr(base_ptr(mptr(obj))),
		type(std::addressof(identity::fetch(obj))) {
		SYNAFIS_ASSERT(ptr != nullptr && type != nullptr);
	}

	/**	\fn hard_ptr(T *obj, std::nothrow_t) noexcept
	 *	\brief Tries to initialize the hard_ptr with obj.
	 *	\tparam The type pointed to by obj.
	 *	\param obj The object to try making a hard_ptr for.
	 */
	template<typename T>
	hard_ptr(T *obj, std::nothrow_t) noexcept : hard_ptr() {
		ptr = base_ptr(mptr(obj));
		if (ptr) {
			type = identity::fetch(ptr, std::nothrow);
			SYNAFIS_ASSERT(type != nullptr);
		}
	}

	/**	\fn hard_ptr(hard_ptr const& other) noexcept
	 *	\brief Copies other.
	 *	\param other The hard_ptr to copy.
	 *	\post other == *this
	 */
	constexpr hard_ptr(hard_ptr const& other) noexcept :
		ptr(other.ptr), type(other.type) {}

	/**	\fn hard_ptr(hard_ptr && other) noexcept
	 *	\brief Moves the content of other to this.
	 *	\param other The hard_ptr to move.
	 *	\post other == nullptr
	 */
	constexpr hard_ptr(hard_ptr && other) noexcept :
		ptr(std::exchange(other.ptr, nullptr)),
		type(std::exchange(other.type, nullptr)) {}

	/**	\fn hard_ptr(soft_ptr const& other)
	 *	\brief Retrieves a hard_ptr from other.
	 *	\param other The soft_ptr to get a hard_ptr from.
	 *	\see get_hard
	 */
	hard_ptr(soft_ptr const& other);

	/**	\fn hard_ptr(Args &&... args)
	 *	\brief Creates a new hard_ptr containing a newly constructed object.
	 *	\tparam T The type to construct.
	 *	\tparam Args Types of parameters to forward to the constructor of T.
	 *	\param args Parameters to forward to the constructor of T.
	 *	\throws Throws std::bad_alloc if memory could not be allocated for the object.
	 *	\details Allocates memory via identity::allocate for the identity of T.
	 *	\details If successful, constructs the object with placement new.
	 */
	template<typename T, typename... Args>
	hard_ptr(Args &&... args) : hard_ptr() {
		construct(std::forward<Args>(args)...);
		SYNAFIS_ASSERT(ptr == nullptr || type != nullptr);
	}

	/**	\fn ~hard_ptr() noexcept
	 *	\brief Default.
	 */
	~hard_ptr() noexcept = default;

	/**	\fn operator=(std::nullptr_t) noexcept
	 *	\brief Initialize with a null pointer and no type.
	 *	\returns *this
	 */
	constexpr hard_ptr &operator=(std::nullptr_t) noexcept {
		if (ptr) {
			ptr = nullptr;
			type = nullptr;
		}
		return *this;
	}

	/**	\fn operator=(soft_ptr const& other)
	 *	\brief Retrieves a hard_ptr from other.
	 *	\param other The soft_ptr to get a hard_ptr from.
	 *	\returns *this
	 *	\see get_hard
	 */
	hard_ptr &operator=(soft_ptr const& other);

	/**	\fn operator=(hard_ptr const& other) noexcept
	 *	\brief Copies other.
	 *	\param other The hard_ptr to copy.
	 *	\returns *this
	 *	\post other == *this
	 */
	hard_ptr &operator=(hard_ptr const& other) noexcept;

	/**	\fn operator=(hard_ptr && other) noexcept
	 *	\brief Moves the content of other to this.
	 *	\param other The hard_ptr to move.
	 *	\returns *this
	 *	\post other == nullptr
	 */
	hard_ptr &operator=(hard_ptr && other) noexcept;

	/**	\fn operator=(T *obj) noexcept
	 *	\brief *this = hard_ptr<T>{obj, std::nothrow}
	 *	\tparam The type pointed to by obj.
	 *	\param obj The object to try setting the hard_ptr for.
	 *	\returns *this
	 */
	template<typename T>
	hard_ptr &operator=(T *obj) noexcept {
		if (obj) {
			ptr = base_ptr(mptr(obj));
			type = ptr ? identity::fetch(ptr, std::nothrow) : nullptr;
			return *this;
		} else {
			return *this = nullptr;
		}
	}

	/**	\fn operator==(std::nullptr_t) const noexcept
	 *	\brief Checks if ptr == nullptr.
	 *	\returns Returns true if ptr == nullptr.
	 */
	constexpr bool operator==(std::nullptr_t) const noexcept {
		return ptr == nullptr;
	}

	/**	\fn operator==(hard_ptr const& other) const noexcept
	 *	\brief Checks if two hard_ptrs point to the same object.
	 *	\param other The hard_ptr to compare with.
	 *	\returns True if the hard_ptrs point to the same object.
	 *	\note "Same" can mean type->equal(ptr, other.ptr) == true.
	 */
	bool operator==(hard_ptr const& other) const noexcept;

	/**	\fn operator!=(T *other) const noexcept
	 *	\brief Checks if other points to the same object as ptr.
	 *	\tparam T The type of the object other points to.
	 *	\param other The pointer to compare with.
	 *	\returns True if other points to the same object as ptr.
	 *	\note "Same" can mean type->equal(ptr, other) == true.
	 */
	template<typename T>
	bool operator==(T *other) const noexcept {
		return *this == hard_ptr{other, std::nothrow};
	}

	/**	\fn operator!=(std::nullptr_t) const noexcept
	 *	\brief Checks if ptr != nullptr.
	 *	\returns Returns true if ptr != nullptr.
	 */
	constexpr bool operator!=(std::nullptr_t) const noexcept {
		return ptr != nullptr;
	}

	/**	\fn operator!=(T *other) const noexcept
	 *	\brief Checks if other does not point to the same object as ptr.
	 *	\tparam T The type of the object other points to.
	 *	\param other The pointer to compare with.
	 *	\returns True if other does not point to the same object as ptr.
	 *	\note "Same" can mean type->equal(ptr, other) == true.
	 */
	template<typename T>
	bool operator!=(T *other) const noexcept {
		return *this != hard_ptr{other, std::nothrow};
	}

	/**	\fn operator!=(hard_ptr const& other) const noexcept
	 *	\brief Checks if two hard_ptrs do not point to the same object.
	 *	\param other The hard_ptr to compare with.
	 *	\returns True if the hard_ptrs do not point to the same object.
	 *	\note "Same" can mean type->equal(ptr, other.ptr) == true.
	 */
	bool operator!=(hard_ptr const& other) const noexcept;

	/**	\fn operator bool() const noexcept
	 *	\brief Converts to bool.
	 *	\returns Returns true if ptr != nullptr.
	 */
	constexpr operator bool() const noexcept {
		return ptr != nullptr;
	}

	/**	\fn operator!() const noexcept
	 *	\brief Converts to bool.
	 *	\returns Returns true if ptr == nullptr.
	 */
	constexpr bool operator!() const noexcept {
		return ptr == nullptr;
	}

	/**	\fn is_typeof() const noexcept
	 *	\brief Checks if the pointer points to an object of type T.
	 *	\tparam T The type to check for.
	 *	\returns Returns true if ptr points to an object of type T.
	 */
	template<typename T>
	bool is_typeof() const noexcept {
		return  type == std::addressof(get_id<T>());
	}

	/**	\fn get() const
	 *	\brief Gets the pointer if it is of type T.
	 *	\tparam T The type to check for.
	 *	\returns Returns ptr cast to type T if type matches.
	 *	\throws Throws std::runtime_error if the type does not match.
	 */
	template<typename T>
	T *get() const {
		if (typeof<T>()) {
			return const_cast<T *>(static_cast<std::remove_cv_t<T> *>(ptr));
		} else {
			throw std::runtime_error{"The types do not match."};
		}
	}

	/**	\fn get(std::nothrow_t) const noexcept
	 *	\brief Gets the pointer if it is of type T.
	 *	\tparam T The type to check for.
	 *	\returns Returns ptr cast to type T if type matches; otherwise, nullptr is returned.
	 */
	template<typename T>
	T *get(std::nothrow_t) const noexcept {
		if (typeof<T>()) {
			return const_cast<T *>(static_cast<std::remove_cv_t<T> *>(ptr));
		} else {
			return static_cast<T *>(nullptr);
		}
	}

	/**	\fn get_ident(std::nothrow_t) const noexcept
	 *	\brief Gets the identity of the object ptr point to.
	 *	\returns type
	 */
	constexpr identity const*get_ident(std::nothrow_t) const noexcept {
		return type;
	}

	/**	\fn get_ident() const
	 *	\brief Gets the identity of the object ptr point to.
	 *	\returns Returns a reference to *type.
	 *	\throws Throws std::logic_error if type == nullptr.
	 *	\pre type != nullptr
	 */
	identity const&get_ident() const;

	/**	\fn construct(Args &&... args)
	 *	\brief Allocates and constructs a new object.
	 *	\tparam T The type to construct.
	 *	\tparam Args Types of parameters to forward to the constructor of T.
	 *	\param args Parameters to forward to the constructor of T.
	 *	\throws Throws std::bad_alloc if memory could not be allocated for the object.
	 *	\details Allocates memory via identity::allocate for the identity of T.
	 *	\details If successful, constructs the object with placement new.
	 */
	template<typename T, typename... Args>
	void construct(Args &&... args) {
		using mtype = typename std::remove_cv_t<T>;
		identity const&id{get_id<T>()};
		mtype *temp{static_cast<mtype *>(id.allocate())};
		if (temp) {
			new (temp) T(std::forward<Args>(args)...);
			ptr = mptr(temp);
			type = std::addressof(id);
		} else {
			throw std::bad_alloc{};
		}
	}

	/**	\fn traverse(void *arg, enumerate_cb cb) const noexcept
	 *	\brief Enumerates ptr if ptr != nullptr.
	 *	\param arg Optional argument to forward to cb.
	 *	\param cb The callback to enumerate pointers.
	 */
	void traverse(void *arg, enumerate_cb cb) const noexcept {
		if (ptr) {
			(*cb)(arg, ptr);
		}
	}

	/**	\fn remap(void *arg, remap_cb cb) noexcept
	 *	\brief Remaps ptr if ptr != nullptr.
	 *	\param arg Optional argument to forward to cb.
	 *	\param cb The callback to remap pointers.
	 */
	void remap(void *arg, remap_cb cb) noexcept {
		if (ptr) {
			ptr = (*cb)(arg, ptr);
		}
	}
};

namespace traits {

/**	\class pointers_type<hard_ptr>
 *	\brief Specialized to indicate that hard_ptr contains a pointer.
 */
template<>
class pointers_type<hard_ptr> :
	public std::integral_constant<bool, true> {};

}

}

#endif
