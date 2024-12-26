#ifndef CTL_META_ENUMTYPE_H
#define CTL_META_ENUMTYPE_H

#include "../namespace.hpp"
#include "basictraits.hpp"
#include "../algorithm/bitwise.hpp"
#include "../meta/logic.hpp"
#include "../meta/pack.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Enum-specific decay facilities.
namespace Decay::Enum {
	/// @brief Decays an enum type to a signed integer type of equal size.
	/// @tparam T Enum type.
	template<Type::Enumerator T>
	using AsSignedInteger = Meta::NthType<(bitWidth(sizeof(T))-1), int8, int16, int32, int64>;

	/// @brief Decays an enum type to an unsigned integer type of equal size.
	/// @tparam T Enum type.
	template<Type::Enumerator T>
	using AsUnsignedInteger = Meta::NthType<(bitWidth(sizeof(T))-1), uint8, uint16, uint32, uint64>;

	/// @brief Decays an enum type to an integer type of equal size.
	/// @tparam T Enum type.
	template<Type::Enumerator T>
	using AsInteger = Meta::DualType<Type::Signed<T>, AsSignedInteger<T>, AsUnsignedInteger<T>>;
}

/// @brief Casts the enum to an integer type of equivalent size and signedness.
/// @tparam T Enum type.
/// @param e Value to cast.
/// @return Value as equivalent integer type.
template<Type::Enumerator T>
constexpr Decay::Enum::AsInteger<T> enumcast(T const& e) {
	return static_cast<Decay::Enum::AsInteger<T>>(e);
}

CTL_NAMESPACE_END

#endif