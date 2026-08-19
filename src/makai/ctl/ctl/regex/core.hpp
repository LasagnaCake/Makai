#ifndef CTL_REGEX_CORE_H
#define CTL_REGEX_CORE_H

#include "../namespace.hpp"
#include "handler.hpp"

CTL_NAMESPACE_BEGIN

namespace Regex {
	/// @brief Replaces content in a string via a regular expression.
	/// @param str String to replace.
	/// @param expr Expression to match.
	/// @param fmt String to replace match with.
	/// @return Replaced string.
	inline String replace(String const& str, String const& expr, String const& fmt) {
		return IHandler::handler().replace(str, expr, fmt);
	}

	/// @brief Checks if the given string contains the regular expression inside it.
	/// @param str String to check.
	/// @param expr Regular expression to match.
	/// @return Whether string contains the expression.
	inline bool contains(String const& str, String const& expr) {
		return IHandler::handler().contains(str, expr);
	}

	/// @brief Checks if the given string fully matches the regular expression.
	/// @param str String to check.
	/// @param expr Regular expression to match.
	/// @return Whether string matches.
	inline bool matches(String const& str, String const& expr) {
		return IHandler::handler().matches(str, expr);
	}

	/// @brief Counts all occurrences of a given regular expression.
	/// @param str String to search in.
	/// @param expr Regular expression to match.
	/// @return Match count.
	inline usize count(String const& str, String const& expr) {
		return IHandler::handler().count(str, expr);
	}

	/// @brief Finds all occurrences of a given regular expression.
	/// @param str String to search in.
	/// @param expr Regular expression to match.
	/// @return List of matches.
	inline List<Match> find(String const& str, String const& expr) {
		return IHandler::handler().find(str, expr);
	}

	/// @brief Finds the first occurrence of a given regular expression.
	/// @param str String to search in.
	/// @param expr Regular expression to match.
	/// @return First match.
	inline Nullable<Match> findFirst(String const& str, String const& expr) {
		return IHandler::handler().findFirst(str, expr);
	}
}

CTL_NAMESPACE_END

#endif // CTL_REGEX_CORE_H
