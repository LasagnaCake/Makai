#ifndef CTL_REGEX_CORE_H
#define CTL_REGEX_CORE_H

#include "../namespace.hpp"
#include "../container/string.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#include <regex>
#pragma GCC diagnostic pop

CTL_NAMESPACE_BEGIN

namespace Regex {
	namespace {
		inline std::string stdstr(String const& expr) {
			return expr.std();
		}

		inline String ctlstr(std::string const& expr) {
			return String(expr);
		}

		inline std::regex makeRegex(String const& expr) {
			return std::regex(stdstr(expr));
		}
	}

	/// @brief Replaces content in a string via a regular expression.
	/// @param str String to replace.
	/// @param expr Expression to match.
	/// @param fmt String to replace match with.
	/// @return Replaced string.
	inline String replace(String const& str, String const& expr, String const& fmt) {
		return std::regex_replace(stdstr(str), makeRegex(expr), stdstr(fmt)).c_str();
	}

	/// @brief Checks if the given string contains the regular expression inside it.
	/// @param str String to check.
	/// @param expr Regular expression to match.
	/// @return Whether string contains the expression.
	inline bool contains(String const& str, String const& expr) {
		std::smatch rm;
		auto cs = stdstr(str);
		return std::regex_search(cs, rm, makeRegex(expr));
	}

	/// @brief Checks if the given string fully matches the regular expression.
	/// @param str String to check.
	/// @param expr Regular expression to match.
	/// @return Whether string matches.
	inline bool matches(String const& str, String const& expr) {
		std::smatch rm;
		auto cs = stdstr(str);
		return std::regex_match(cs, rm, makeRegex(expr));
	}

	/// @brief Regex match.
	struct Match {
		/// @brief Match position.
		ssize	position;
		/// @brief Match contents.
		String	match;
	};

	/// @brief Finds all occurrences of a given regular expression.
	/// @param str String to search in.
	/// @param expr Regular expression to match.
	/// @return List of matches.
	inline List<Match> find(String const& str, String const& expr) {
		std::smatch rm;
		List<Match> result;
		auto cs = stdstr(str);
		std::regex_search(cs, rm, makeRegex(expr));
		for (usize i = 1; i < rm.size()-1; ++i)
			result.pushBack(Match{
				rm.position(i),
				ctlstr(rm[i].str())
			});
		return result;
	}

	/// @brief Finds the first occurrence of a given regular expression.
	/// @param str String to search in.
	/// @param expr Regular expression to match.
	/// @return First match.
	inline Match findFirst(String const& str, String const& expr) {
		std::smatch rm;
		auto cs = stdstr(str);
		std::regex_search(cs, rm, makeRegex(expr));
		return Match{rm.position(1), ctlstr(rm[1].str())};
	}
}

CTL_NAMESPACE_END

#endif // CTL_REGEX_CORE_H
