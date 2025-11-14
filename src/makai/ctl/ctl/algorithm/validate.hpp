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

/// @brief Returns whether the given character is a bracket character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a bracket character.
template <Type::ASCII TChar>
constexpr bool isBracketChar(TChar const& c) noexcept {
	return (
		c == '{' || c == '}'
	||	c == '[' || c == ']'
	);
}

/// @brief Returns whether the given character is a parentheses character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a parentheses character.
template <Type::ASCII TChar>
constexpr bool isParensChar(TChar const& c) noexcept {
	return (c == '(' || c == ')');
}

/// @brief Returns whether the given character is a quote character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a quote character.
template <Type::ASCII TChar>
constexpr bool isQuoteChar(TChar const& c) noexcept {
	return (c == '"' || c == '\'');
}

/// @brief Returns whether the given character is a scope opener/closer character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a scope character.
template <Type::ASCII TChar>
constexpr bool isScopeChar(TChar const& c) noexcept {
	return isParensChar(c) || isBracketChar(c) || isQuoteChar(c) || c == '<' || c == '>';
}

/// @brief Returns whether the given character is a number character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a number character.
template <Type::ASCII TChar>
constexpr bool isNumberChar(TChar const& c) noexcept {
	return (c >= '0' && c <= '9');
}

/// @brief Returns whether the given character is a uppercase letter character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a upercase letter character.
template <Type::ASCII TChar>
constexpr bool isUppercaseChar(TChar const& c) noexcept {
	return (c >= 'A' && c <= 'Z');
}

/// @brief Returns whether the given character is a lowercase letter character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a lowercase letter character.
template <Type::ASCII TChar>
constexpr bool isLowercaseChar(TChar const& c) noexcept {
	return (c >= 'a' && c <= 'z');
}

/// @brief Returns whether the given character is a letter character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is a letter character.
template <Type::ASCII TChar>
constexpr bool isLetterChar(TChar const& c) noexcept {
	return isUppercaseChar(c) || isLowercaseChar(c);
}

/// @brief Returns whether the given character is an alphanumeric character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is an alphanumeric character.
template <Type::ASCII TChar>
constexpr bool isAlphanumericChar(TChar const& c) noexcept {
	return isLetterChar(c) || isNumberChar(c);
}

/// @brief Returns whether the given character is a valid identifier name character.
/// @tparam TChar Character type.
/// @param c Character to check.
/// @return Whether it is an identifier name character.
template <Type::ASCII TChar>
constexpr bool isIdentifierNameChar(TChar const& c) noexcept {
	return isAlphanumericChar(c) || c == '_';
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_VALIDATE_H
