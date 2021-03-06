
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

/**	\defgroup gc_traits Traits
 *	\brief Group containing type traits for types allocated by the GC.
 *	\ingroup gc_interface
 */

/**	\file src/gc/traits.hpp
 *	\brief Type traits for types that the garbage collector will interact with.
 *	\ingroup gc_traits
 */

/**	\namespace gc::traits
 *	\brief Defines garbage collector type traits.
 *	\ingroup gc_traits
 */

#include "callbacks.hpp"

#ifndef SYNAFIS_GC_TRAITS_HPP
#define SYNAFIS_GC_TRAITS_HPP
#pragma once

#include <bitset>
#include <type_traits>

namespace gc {

namespace traits {

/** \addtogroup gc_traits Traits
 *	@{
 */

/**	\typedef flag_type
 *	\brief The type for holding flags summarizing various traits to inform the collector about.
 */
typedef std::bitset<5> flag_type;

/**	\class moveable_type
 *	\brief Trait to check if the garbage collector can move objects.
 *	\tparam T The type to check for being movable by the collector.
 *	\details All types that can be trivially copied are considered movable by default.
 *	\details
 *	\details Other types should have a specialization defined.
 *	\see movable\<T\>
 */
template<typename T>
class moveable_type : public std::is_trivially_copyable<T>
{};

/**	\var movable
 *	\brief A boolean value indicating if objects of a type can be moved by the collector.
 *	\tparam T The type the value is for. Will be stripped of const and volatile qualifiers.
 *	\details If true while relocator\<T\> == nullptr, then it means objects of
 *	\details the type may be copied via simply copying the contents of memory.
 *	\details
 *	\details If true while relocator\<T\> != nullptr, then it means objects of
 *	\details the type will be relocated by relocator\<T\>.
 *	\details
 *	\details If false then objects of the type cannot be moved by the collector.
 *	\note To make a type movable, specialize movable_type\<T\>.
 *	\see movable_type\<T\>
 */
template<typename T>
inline constexpr bool const movable{moveable_type<std::remove_cv_t<T>>::value};

/**	\class readonly_type
 *	\brief Trait to check if objects allow mutation.
 *	\tparam T The type to check for being immutable.
 *	\details Types with a const qualifier are readonly be default.
 *	\details
 *	\details Other types should have a specialization defined if they
 *	\details cannot be mutated even in the absence of a const qualifier.
 *	\details
 *	\details Other types should have a specialization defined.
 *	\see readonly\<T\>
 */
template<typename T>
class readonly_type : public std::is_const<T>
{};

/**	\var readonly
 *	\brief A boolean value indicating if objects of a type are immutable.
 *	\tparam T The type the value is for. Will be stripped of the volatile qualifier but not const.
 *	\details When types are labelled as being immutable, certain garbage collector
 *	\details optimizations become possible.
 *	\details
 *	\details When true equalizer\<T\> must not be nullptr.
 *	\details
 *	\details The modification of pointers due to remapping is not considered
 *	\details mutation for the purposes of this variable.
 *	\note To make a type readonly, specialize readonly_type\<T\>.
 *	\see readonly_type\<T\>
 */
template<typename T>
inline constexpr bool const readonly{readonly_type<std::remove_volatile_t<T>>::value};

/**	\class pointers_type
 *	\brief Trait to check if objects of a type may hold pointer to collector owned objects.
 *	\tparam T The type to check for being immutable.
 *	\details All types are listed as not holding pointers by default.
 *	\details
 *	\details Other types should have a specialization defined.
 *	\see pointers\<T\>
 */
template<typename T>
class pointers_type : public std::integral_constant<bool, false>
{};

/**	\var pointers
 *	\brief A boolean value indicating if objects of a type contain pointers to other objects.
 *	\tparam T The type the value is for. Will be stripped of const and volatile qualifiers.
 *	\details When true, neither traverser nor relocator may be nullptr.
 *	\note To mark a type as containing pointers, specialize pointers_type\<T\>.
 *	\see pointers_type\<T\>
 */
template<typename T>
inline constexpr bool const pointers{pointers_type<std::remove_cv_t<T>>::value};

/**	\class finalize_type
 *	\brief Trait for getting the finalize_cb for a type.
 *	\tparam T The type to set the callback for.
 *	\tparam Enable Used to enable specializations with std::enable_if_t.
 *	\details For types that are trivially destructible, the value of the pointer will be nullptr.
 *	\details
 *	\details Other types will have it set to a lambda expression that calls the destructor.
 *	\note Only specialize for types that preform clean up outside of the destructor.
 *	\see finalizer\<T\>
 */
template<typename T, typename Enable = void>
class finalize_type;

/**
 *	\brief Specialization for when there is no finalizer.
 *	\tparam T The type to set the callback for.
 *	\see finalize_type\<T, Enable\>
 */
template<typename T>
class finalize_type<T, std::enable_if_t<std::is_trivially_destructible_v<T>>>
{
public:
	/**	\var value
	 *	\brief There is no finalize callback needed.
	 */
	static constexpr finalize_cb const value{nullptr};
};

/**
 *	\brief Specialization for when the finalizer calls the type's destructor.
 *	\tparam T The type to set the callback for.
 *	\see finalize_type\<T, Enable\>
 */
template<typename T>
class finalize_type<T,
	std::enable_if_t<(std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>)>>
{
private:
	/**
	 *	\brief The callback.
	 *	\param obj The object being finalized.
	 */
	static void callabck(void *obj) noexcept
	{
		SYNAFIS_ASSERT(obj != nullptr);
		static_cast<T *>(obj)->~T();
	}

public:
	/**	\var value
	 *	\brief The default finalize callback that calls the type's destructor.
	 */
	static constexpr finalize_cb const value{callback};
};

/**	\var finalizer
 *	\brief A pointer to the finalize_cb for a type.
 *	\tparam T The type the value is for. Will be stripped of const and volatile qualifiers.
 *	\see finalize_type\<T\>
 */
template<typename T>
inline constexpr finalize_cb const finalizer{finalize_type<std::remove_cv_t<T>>::value};

/**	\class traverse_type
 *	\brief Trait for getting the traverse_cb for a type.
 *	\tparam T The type to set the callback for.
 *	\tparam Enable Used to enable specializations with std::enable_if_t.
 *	\details For types that do not contain pointers, set to nullptr.
 *	\details
 *	\details For other types, call the member function traverse(void *, enumerate_cb).
 *	\note Specialize for types lacking a traverse member.
 *	\see traverser\<T\>
 *	\see pointers\<T\>
 */
template<typename T, typename Enable = void>
class traverse_type;

/**
 *	\brief Specialization for types that do not contain pointers.
 *	\tparam T The type to set the callback for.
 *	\see traverse_type\<T, Enable\>
 */
template<typename T>
class traverse_type<T, std::enable_if_t<(!pointers<T>)>>
{
public:
	/**	\var value
	 *	\brief There is no traversal callback needed.
	 */
	static constexpr traverse_cb const value{nullptr};
};

/**
 *	\brief Specialization for types that contain pointers.
 *	\tparam T The type to set the callback for.
 *	\see traverse_type\<T, Enable\>
 */
template<typename T>
class traverse_type<T, std::enable_if_t<(pointers<T>)>>
{
private:
	/**
	 *	\brief The callback.
	 *	\param obj The object being traversed.
	 *	\param data Data to pass to cb.
	 *	\param cb A callback to report pointers to the collector.
	 */
	static void callback(void const *obj, void *data, enumerate_cb cb) noexcept
	{
		SYNAFIS_ASSERT(obj != nullptr);
		SYNAFIS_ASSERT(cb != nullptr);
		static_cast<T const *>(obj)->traverse(data, cb);
	}

public:
	/**	\var value
	 *	\brief The default traversal callback that calls the type's member traverse.
	 */
	static constexpr traverse_cb const value{callback};
};

/**	\var traverser
 *	\brief The pointer to the function for enumerating pointers in objects of the type.
 *	\tparam T The type the value is for. Will be stripped of const and volatile qualifiers.
 *	\details Types that do not contain pointers owned by the collector do not need
 *	\details this callback.
 *	\see traverser_type\<T\>
 */
template<typename T>
inline constexpr traverse_cb const traverser{traverse_type<std::remove_cv_t<T>>::value};

/**	\fn transfer(T &&orig, T &dest) noexcept
 *	\brief The function used to specify how to move objects of a type.
 *	\tparam T The type to transfer.
 *	\tparam Enable Used to enable specializations with std::enable_if_t.
 *	\param orig The original location.
 *	\param dest The destination location.
 *	\details Used in defining relocate_type\<T\>.
 *	\details
 *	\details If the type is nothrow move assignable, will use move assignment.
 *	\details
 *	\details If the type is nothrow copy assignable, will use copy assignment.
 *	\details
 *	\details For other cases, a specialization must be defined.
 *	\see relocator_type\<T\>
 */
template<typename T>
void transfer(T &&orig, T &dest) noexcept;

/**
 *	\brief Specialization for when move assignment is usable.
 *	\tparam T The type to transfer.
 *	\param orig The original location.
 *	\param dest The destination location.
 *	\see transfer\<T, Enable\>()
 */
template<typename T>
inline std::enable_if_t<std::is_nothrow_move_assignable_v<T>> transfer(T &&orig, T &dest) noexcept
{
	dest = std::forward<T>(orig);
}

/**
 *	\brief Specialization for when copy assignment is usable but not move assignment.
 *	\tparam T The type to transfer.
 *	\param orig The original location.
 *	\param dest The destination location.
 *	\see transfer\<T, Enable\>()
 */
template<typename T>
inline std::enable_if_t<!std::is_nothrow_move_assignable_v<T> &&
						std::is_nothrow_copy_assignable_v<T>>
	transfer(T &&orig, T &dest) noexcept
{
	T const &temp{orig};
	dest = temp;
}

/**	\class relocate_type
 *	\brief Trait for getting the relocate_cb for a type.
 *	\tparam T The type to set the callback for.
 *	\tparam Enable Used to enable specializations with std::enable_if_t.
 *	\details If pointers\<T\>, the defined callback will call the member function remap(void *, remap_cb).
 *	\details
 *	\details For those that are movable\<T\> and need a relocate_cb, the callback calls
 *	\details transfer(T &&, T &). The transfer occurs before the remap if both are preformed.
 *	\details
 *	\details For types !pointers\<T\> and either trivially copyable or !movable\<T\> the
 *	\details callback is set to nullptr.
 *	\note Specialize for types lacking a remap member.
 *	\see relocator\<T\>
 *	\see pointers\<T\>
 *	\see transfer(T &&, T &) noexcept
 */
template<typename T, typename Enable = void>
class relocate_type;

/**
 *	\brief Specialization for when there is no relocation callback.
 *	\tparam T The type to set the callback for.
 *	\see relocate_type\<T, Enable\>
 */
template<typename T>
class relocate_type<T,
	std::enable_if_t<(!pointers<T> && (std::is_trivially_copyable_v<T> || !movable<T>))>>
{
public:
	/**	\var value
	 *	\brief There is no relocation callback available.
	 */
	static constexpr relocate_cb const value{nullptr};
};

/**
 *	\brief Specialization for when the callback utilizes transfer().
 *	\tparam T The type to set the callback for.
 *	\see relocate_type\<T, Enable\>
 */
template<typename T>
class relocate_type<T,
	std::enable_if_t<(!pointers<T> && !std::is_trivially_copyable_v<T> && movable<T>)>>
{
private:
	/**
	 *	\brief The callback.
	 *	\param orig The original address.
	 *	\param dest The destination address.
	 *	\param data Data for the callback.
	 *	\param cb The remap callback from the collector.
	 */
	static void callback(void *orig, void *dest, void *data, remap_cb cb) noexcept
	{
		SYNAFIS_ASSERT(orig != nullptr);
		SYNAFIS_ASSERT(dest != nullptr);
		SYNAFIS_ASSERT(orig != dest);
		transfer(std::move(*static_cast<T *>(orig)), *static_cast<T *>(dest));
	}

public:
	/**	\var value
	 *	\brief The default relocate callback that calls the transfer\<T\>.
	 */
	static constexpr relocate_cb const value{callback};
};

/**
 *	\brief Specialization for when the callback calls remap().
 *	\tparam T The type to set the callback for.
 *	\see relocate_type\<T, Enable\>
 */
template<typename T>
class relocate_type<T, std::enable_if_t<(pointers<T> && !movable<T>)>>
{
private:
	/**
	 *	\brief The callback.
	 *	\param orig The original address.
	 *	\param dest The destination address.
	 *	\param data Data for the callback.
	 *	\param cb The remap callback from the collector.
	 */
	static void callback(void *orig, void *dest, void *data, remap_cb cb) noexcept
	{
		SYNAFIS_ASSERT(dest != nullptr);
		SYNAFIS_ASSERT(cb != nullptr);
		SYNAFIS_ASSERT(orig == dest);
		static_cast<T *>(dest)->remap(data, cb);
	}

public:
	/**	\var value
	 *	\brief The default traversal callback that calls the type's member remap.
	 *	\details This version does not move the object.
	 */
	static constexpr relocate_cb const value{callback};
};

/**
 *	\brief Specialization for when the callback utilizes transfer() and calls remap().
 *	\tparam T The type to set the callback for.
 *	\see relocate_type\<T, Enable\>
 */
template<typename T>
class relocate_type<T, std::enable_if_t<(pointers<T> && movable<T>)>>
{
private:
	/**
	 *	\brief The callback.
	 *	\param orig The original address.
	 *	\param dest The destination address.
	 *	\param data Data for the callback.
	 *	\param cb The remap callback from the collector.
	 */
	static void callback(void *orig, void *dest, void *data, remap_cb cb) noexcept
	{
		SYNAFIS_ASSERT(orig != nullptr);
		SYNAFIS_ASSERT(dest != nullptr);
		SYNAFIS_ASSERT(cb != nullptr);
		T &ref{*static_cast<T *>(dest)};
		if (orig != dest) { transfer(std::move(*static_cast<T *>(orig)), ref); }
		static_cast<T *>(dest)->remap(data, cb);
	}

public:
	/**	\var value
	 *	\brief The default traversal callback that calls the type's member remap.
	 *	\details This version moves the object to a new address with move assignment
	 *	\details if the collector provides a new address.
	 */
	static constexpr relocate_cb const value{callback};
};

/**	\var relocator
 *	\brief The pointer to the function for relocating pointers in objects of the type.
 *	\tparam T The type the value is for. Will be stripped of const and volatile qualifiers.
 *	\details For types that needs a specialized move operation or contain
 *	\details pointers to remap.
 *	\see relocator_type\<T\>
 */
template<typename T>
inline constexpr relocate_cb const relocator{relocate_type<std::remove_cv_t<T>>::value};

/**	\fn compare(T const& lhs, T const& rhs) noexcept
 *	\brief The function used to check if two objects will always be equal.
 *	\tparam T The type being compared.
 *	\param lhs The left hand side of the comparison.
 *	\param rhs The right hand side of the comparison.
 *	\details Used in defining equality_type\<T\>.
 *	\details
 *	\details Defaults to lhs == rhs.
 *	\see equality_type\<T\>
 */
template<typename T>
bool compare(T const &lhs, T const &rhs) noexcept
{
	return lhs == rhs;
}

/**	\class equality_type
 *	\brief Trait for getting the equality_cb for a type.
 *	\tparam T The type to set the callback for.
 *	\tparam Enable Used to enable specializations with std::enable_if_t.
 *	\details If pointers\<T\>, the defined callback will call the member function remap(void *, remap_cb).
 *	\details
 *	\details For those that are movable\<T\> and need a relocate_cb, the callback calls
 *	\details transfer(T &&, T &). The transfer occurs before the remap if both are preformed.
 *	\details
 *	\details For types !pointers\<T\> and either trivially copyable or !movable\<T\> the
 *	\details callback is set to nullptr.
 *	\note Specialize compare() and readonly\<T\> instead.
 *	\see equalizer\<T\>
 *	\see readonly\<T\>
 *	\see compare(T const&, T const&) noexcept
 */
template<typename T, typename Enable = void>
class equality_type;

/**
 *	\brief Specialization for when there is not equality callback.
 *	\tparam T The type to set the callback for.
 *	\see equality_type\<T, Enable\>
 */
template<typename T>
class equality_type<T, std::enable_if_t<(!readonly<T>)>>
{
public:
	/**	\var value
	 *	\brief There is no equality callback available.
	 */
	static constexpr equality_cb const value{nullptr};
};

/**
 *	\brief Specialization for when the callback utilizes compare().
 *	\tparam T The type to set the callback for.
 *	\see equality_type\<T, Enable\>
 */
template<typename T>
class equality_type<T, std::enable_if_t<(readonly<T>)>>
{
private:
	/**
	 *	\brief The callback.
	 *	\param lhs The left hand side of the comparison.
	 *	\param rhs The right hand side of the comparison.
	 *	\returns Returns lhs == rhs.
	 */
	static bool callback(void const *lhs, void const *rhs) noexcept
	{
		SYNAFIS_ASSERT(lhs != rhs);
		SYNAFIS_ASSERT(lhs != nullptr);
		SYNAFIS_ASSERT(rhs != nullptr);
		return compare(
			*static_cast<std::add_const_t<T> *>(lhs), *static_cast<std::add_const_t<T> *>(rhs));
	};

public:
	/**	\var value
	 *	\brief The default equality callback that calls the compare\<T\>.
	 */
	static constexpr equality_cb const value{callback};
};

/**	\var equalizer
 *	\brief The pointer to the function for checking if two objects will always be equal.
 *	\tparam T The type the value is for. Will be stripped of the volatile qualifier.
 *	\details The concept here is to treat immutable objects that will always be equal
 *	\details as the same object.
 *	\details
 *	\details By doing so, the collector is free to detect such objects and remap all
 *	\details pointers to any of them to just one object. Then the memory of the other
 *	\details objects may be released.
 *	\note To be practical an additional callback to get a hash for immutable objects
 *	\note to quickly reject some possibilities.
 *	\see equality_type\<T\>
 */
template<typename T>
inline constexpr equality_cb const equalizer{equality_type<std::remove_volatile_t<T>>::value};

/**	\fn get_flags() noexcept
 *	\brief Gets a set of flags summarizing trait information to pass to the collector.
 *	\tparam T The type to get a summary for.
 *	\details The flags from least to most significant are:
 *	\details - movable\<T\>
 *	\details - finalizer\<T\> != nullptr
 *	\details - pointers\<T\>
 *	\details - relocator\<T\> != nullptr
 *	\details - readonly\<T\>
 *	\see flag_type
 */
template<typename T>
constexpr inline flag_type get_flags() noexcept
{
	return flag_type{(movable<T> ? 0x1u : 0u) | (finalizer<T> != nullptr ? 0x2u : 0u) |
					 (pointers<T> ? 0x4u : 0u) | (relocator<T> != nullptr ? 0x8u : 0u) |
					 (readonly<T> ? 0x10u : 0u)};
}
//!	@}
}
}

#endif
