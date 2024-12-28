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

/// @brief Returns whether the given character is a whitespace character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a whitespace character.
template <Type::ASCII TChar>
constexpr bool isSpaceChar(TChar const& c) noexcept {
	return (
		c == ' '
	||	c == '\t'
	||	c == '\v'
	||	c == '\n'
	||	c == '\r'
	||	c == '\f'
	);
}

/// @brief Returns whether the given character is a null character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a null character.
template <Type::ASCII TChar>
constexpr bool isNullChar(TChar const& c) noexcept {
	return (c == '\0');
}

/// @brief Returns whether the given character is a null or whitespace character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a null or whitespace character.
template <Type::ASCII TChar>
constexpr bool isNullOrSpaceChar(TChar const& c) noexcept {
	return isNullChar(c) || isSpaceChar(c);
}

/// @brief Returns whether the given character is a new line character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a new line character.
template <Type::ASCII TChar>
constexpr bool isNewLineChar(TChar const& c) noexcept {
	return (c == '\n' || c == '\r' || c == '\f');
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_VALIDATE_H
