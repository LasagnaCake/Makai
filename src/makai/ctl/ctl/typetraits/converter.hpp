#ifndef CTL_TYPETRAITS_CONVERTER_H
#define CTL_TYPETRAITS_CONVERTER_H

#include "../ctypes.hpp"
#include "containers.hpp"
#include "signedness.hpp"
#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief General implementations.
namespace Impl {
	template<class T> struct NonReferenceType:		TypeContainer<T> {};
	template<class T> struct NonReferenceType<T&>:	TypeContainer<T> {};
	template<class T> struct NonReferenceType<T&&>:	TypeContainer<T> {};

	template<class T> struct NonConstType:					TypeContainer<T> {};
	template<class T> struct NonConstType<T const>:			TypeContainer<T> {};

	template<class T> struct NonVolatileType:				TypeContainer<T> {};
	template<class T> struct NonVolatileType<T volatile>:	TypeContainer<T> {};

	template<class T> struct NonCVType:						TypeContainer<T> {};
	template<class T> struct NonCVType<T const>:			TypeContainer<T> {};
	template<class T> struct NonCVType<T volatile>:			TypeContainer<T> {};
	template<class T> struct NonCVType<T const volatile>:	TypeContainer<T> {};

	template<class T> struct NormalType:	TypeContainer<typename NonCVType<typename NonReferenceType<T>::Type>::Type> {};

	template<class T> struct NonPointerType:		TypeContainer<typename NonCVType<T>::Type> {};
	template<class T> struct NonPointerType<T*>:	TypeContainer<typename NonCVType<T>::Type> {};

	template<class T> struct ReferenceType:			TypeContainer<T&> {};
	template<class T> struct ReferenceType<T&>:		TypeContainer<T&> {};
	template<class T> struct ReferenceType<T&&>:	TypeContainer<T&> {};

	template<class T> struct TemporaryType:			TypeContainer<T&&> {};
	template<class T> struct TemporaryType<T&>:		TypeContainer<T&&> {};
	template<class T> struct TemporaryType<T&&>:	TypeContainer<T&&> {};

	template<class T> struct ConstantType:		TypeContainer<T const>			{};
	template<class T> struct VolatileType:		TypeContainer<T volatile>		{};
	template<class T> struct ConstVolatileType:	TypeContainer<T const volatile>	{};

	template<class T>			struct ArrayElementType:		TypeContainer<T>	{};
	template<class T>			struct ArrayElementType<T[]>:	TypeContainer<T>	{};
	template<class T, usize N>	struct ArrayElementType<T[N]>:	TypeContainer<T>	{};

	template<class T>			struct NonExtentType:		TypeContainer<T>								{};
	template<class T>			struct NonExtentType<T[]>:	TypeContainer<typename NonExtentType<T>::Type>	{};
	template<class T, usize N>	struct NonExtentType<T[N]>:	TypeContainer<typename NonExtentType<T>::Type>	{};

	template<class T>			struct ArrayType:		TypeContainer<T[]> {};
	template<class T>			struct ArrayType<T[]>:	TypeContainer<T[]> {};
	template<class T, usize N>	struct ArrayType<T[N]>:	TypeContainer<T[]> {};

	template<class T, usize N>	struct FixedArrayType:			TypeContainer<T[N]> {};
	template<class T, usize N>	struct FixedArrayType<T[], N>:	TypeContainer<T[N]> {};
	template<class T, usize N>	struct FixedArrayType<T[N], N>:	TypeContainer<T[N]> {};

	template<class T>					struct ReturnType:				TypeContainer<T> {};
	template<class T, class... Args>	struct ReturnType<T(Args...)>:	TypeContainer<T> {};
}

/// @brief Removes reference qualifier from type.
template<class T> using AsNonReference = typename Impl::NonReferenceType<T>::Type;

/// @brief Removes `const` qualifier from type.
template<class T> using AsNonConst = typename Impl::NonConstType<T>::Type;

/// @brief Removes `volatile` qualifier from type.
template<class T> using AsNonVolatile = typename Impl::NonVolatileType<T>::Type;

/// @brief Removes `const` and `volatile` qualifier from type.
template<class T> using AsNonCV = typename Impl::NonCVType<T>::Type;

/// @brief Removes `const`, `volatile` and reference qualifier from type.
template<class T> using AsNormal = typename Impl::NormalType<T>::Type;

/// @brief Removes pointer qualifier from type.
template<class T> using AsNonPointer = typename Impl::NonPointerType<T>::Type;

/// @brief Adds pointer qualifier to type.
template<class T> using AsPointer = AsNonReference<T>*;

/// @brief Adds lvalue reference (standard refefence) qualifier to type.
template<class T> using AsReference	= typename Impl::ReferenceType<T>::Type;

/// @brief Adds rvalue reference (temporary reference) qualifier to type.
template<class T> using AsTemporary	= typename Impl::TemporaryType<T>::Type;

/// @brief Adds `const` qualifier to type.
template<class T> using AsConstant		= typename Impl::ConstantType<T>::Type;
/// @brief Adds `volatile` qualifier to type.
template<class T> using AsVolatile		= typename Impl::VolatileType<T>::Type;
/// @brief Adds `const` and `volatile` qualifier to type.
template<class T> using AsConstVolatile	= typename Impl::ConstVolatileType<T>::Type;

/// @brief Gets the underlying element of a C-style array type.
template<class T> using AsArrayElement = typename Impl::ArrayElementType<T>::Type;

/// @brief Gets the underlying element of a C-style array-of-arrays type.
template<class T> using AsNonExtent	= typename Impl::NonExtentType<T>::Type;

/// @brief Makes a C-style array type from a given type.
template<class T> using AsArray	= typename Impl::ArrayType<T>::Type;

/// @brief Makes a C-style fixed array type from a given type.
template<class T, usize N> using AsFixedArray = typename Impl::FixedArrayType<T, N>::Type;

/// @brief Gets the return type of a function type.
template<class T> using AsReturn = typename Impl::ReturnType<T>::Type;

/// @brief Enables the usage of the return type of member functions without going through the type's constructor.
template<typename T> AsTemporary<T> declval();

/// @brief Gets a given iterator type's underlying referenced value type.
template<typename T>
using AsIteratorValue = decltype(*declval<T>());

CTL_NAMESPACE_END

#endif // CTL_TYPETRAITS_CONVERTER_H
