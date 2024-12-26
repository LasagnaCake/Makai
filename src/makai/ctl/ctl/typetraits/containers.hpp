#ifndef CTL_TYPETRAITS_CONTAINERS_H
#define CTL_TYPETRAITS_CONTAINERS_H

#include "../ctypes.hpp"
#include "typecontainer.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Void type. Always decays to `void`.
template <class...>
using VoidType = void;

/// @brief Compile-time value constant.
/// @tparam T Type of value.
/// @tparam V Value to assign.
template<class T, T V> struct ValueConstant {
	typedef T DataType;

	constexpr static DataType value = V;
};

/// @brief Type constraints.
namespace Type {}

/// @brief Trait-specific type constraints.
namespace Type::Trait {
	/// @brief Type must hold a compile-time value constant.
	template<class C>
	concept ValueConstant =
		requires {
			typename C::DataType;
			C::value;
		} && (C::value == ::CTL::ValueConstant<typename C::DataType, C::value>::value)
	;
}

/// @brief Compile-time boolean constant.
/// @tparam V Value to assign.
template<bool V> struct BooleanConstant: ValueConstant<bool, V> {
		template <typename... Args>
		requires (... && Type::Trait::ValueConstant<Args>)
		struct And: BooleanConstant<(... && Args::value)> {};

		template <typename... Args>
		requires (... && Type::Trait::ValueConstant<Args>)
		struct Or: BooleanConstant<(... || Args::value)> {};
};

/// @brief Compile-time size type constant.
/// @tparam V Value to assign.
template<usize V> struct SizeConstant: ValueConstant<usize, V> {};

/// @brief Compile-time true type.
struct TrueType:	BooleanConstant<true>	{};
/// @brief Compile-time false type.
struct FalseType:	BooleanConstant<false>	{};

CTL_NAMESPACE_END

#endif // CTL_TYPETRAITS_CONTAINERS_H
