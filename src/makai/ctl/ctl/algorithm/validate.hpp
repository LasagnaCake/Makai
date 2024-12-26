#ifndef CTL_ALGORITHM_VALIDATE_H
#define CTL_ALGORITHM_VALIDATE_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "transform.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Returns whether the given character is a hex character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a hex character.
template<Type::ASCII TChar>
constexpr bool isHexChar(TChar const& c) noexcept {
	if (c >= '0' && c <= '9') return true;
	if (c >= 'A' && c <= 'F') return true;
	if (c >= 'a' && c <= 'f') return true;
	return false;
}

/// @brief Returns whether the given character is a null or whitespace character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a null or whitespace character.
template <Type::ASCII TChar>
constexpr bool isNullOrSpaceChar(TChar const& c) noexcept {
	return (
		c == '\0'
	||	c == ' '
	||	c == '\t'
//	||	c == '\n'
	);
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_VALIDATE_H
