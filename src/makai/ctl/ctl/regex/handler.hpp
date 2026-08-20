#ifndef CTL_REGEX_HANDLER_H
#define CTL_REGEX_HANDLER_H

#include "../namespace.hpp"
#include "../container/strings/string.hpp"
#include "../container/nullable.hpp"

CTL_NAMESPACE_BEGIN

namespace Regex {
	/// @brief Regex match.
	struct Match {
		/// @brief Match position.
		ssize	position;
		/// @brief Match contents.
		String	match;
	};

	struct IHandler {
		// @brief Destructor.
		constexpr virtual ~IHandler() {}

		/// @brief Replaces content in a string via a regular expression.
		/// @param str String to replace.
		/// @param expr Expression to match.
		/// @param fmt String to replace match with.
		/// @return Replaced string.
		constexpr virtual String replace(String const& str, String const& expr, String const& fmt) = 0;

		/// @brief Checks if the given string contains the regular expression inside it.
		/// @param str String to check.
		/// @param expr Regular expression to match.
		/// @return Whether string contains the expression.
		constexpr virtual bool contains(String const& str, String const& expr) = 0;

		/// @brief Checks if the given string fully matches the regular expression.
		/// @param str String to check.
		/// @param expr Regular expression to match.
		/// @return Whether string matches.
		constexpr virtual bool matches(String const& str, String const& expr) = 0;

		/// @brief Counts all occurrences of a given regular expression.
		/// @param str String to search in.
		/// @param expr Regular expression to match.
		/// @return Match count.
		constexpr virtual usize count(String const& str, String const& expr) = 0;

		/// @brief Finds all occurrences of a given regular expression.
		/// @param str String to search in.
		/// @param expr Regular expression to match.
		/// @return List of matches.
		constexpr virtual List<Match> find(String const& str, String const& expr) = 0;

		/// @brief Finds the first occurrence of a given regular expression.
		/// @param str String to search in.
		/// @param expr Regular expression to match.
		/// @return First match.
		constexpr virtual Nullable<Match> findFirst(String const& str, String const& expr) = 0;

		static IHandler& handler() {
			return *globalHandler;
		}

		static void setHandler(IHandler& handler) {
			globalHandler = &handler;
		}

		template <Type::Subclass<IHandler> T>
		static T& makeHandler() {
			static T handler;
			setHandler(handler);
			return handler;
		}

	private:
		inline static ref<IHandler> globalHandler = nullptr;
	};


}

CTL_NAMESPACE_END

#endif // CTL_REGEX_CORE_H
