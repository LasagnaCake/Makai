#ifndef CTL_EX_DATA_PARSE_H
#define CTL_EX_DATA_PARSE_H

#include "value.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Data-specific type constraints.
namespace Type::Data {
	/// @brief Type must be a valid string parser that returns `TError`.
	template <class T, class TError>
	concept StringParser = requires (T parser) {
		{parser.tryParse(::CTL::Ex::Data::Value::StringType())} -> Type::Equal<Result<::CTL::Ex::Data::Value, TError>>;
	};

	/// @brief Type must be a valid byte parser that returns `TError`.
	template <class T, class TError>
	concept ByteParser = requires (T parser) {
		{parser.tryParse(::CTL::Ex::Data::Value::ByteListType())} -> Type::Equal<Result<::CTL::Ex::Data::Value, TError>>;
	};
}

/// @brief Data exchange format facilities.
namespace Data {
	/// @brief String parse error.
	struct StringParseError {
		/// @brief Error position.
		struct Position {
			/// @brief Point in the string the error occurred.
			usize	at;
			/// @brief Line in the file.
			usize	line;
			/// @brief Column in the file.
			usize	column;
		} position;
		/// @brief Error message.
		String				what;
		/// @brief Content where the error occurred.		
		Value::StringType	content;

		/// @brief Throws an exception detailing the error.
		/// @throw Error::FailedAction.
		[[noreturn]]
		void raise() const {
			throw Error::FailedAction(
				toString(
					what,
					"\nAt:",
					"\nLINE   : ", position.line,
					"\nCOLUMN : ", position.column,
					"\n\n--> (", content, ")"
				)
			);
		}
	};

	/// @brief Byte parse error.
	struct ByteParseError {
		/// @brief Where in the byte data it occurred.
		usize	at;
		/// @brief Error message.
		String	what;

		/// @brief Throws an exception detailing the error.
		/// @throw Error::FailedAction.
		[[noreturn]]
		void raise() const {
			throw Error::FailedAction(toString(what, "\nAt BYTE [", at, "]"));
		}
	};

	/// @brief Parses a string with a given parser.
	/// @tparam T Parser type.
	/// @param str String to parse.
	/// @return Parsed data.
	/// @throw Error::FailedAction on parse failures.	
	template <Type::Data::StringParser<StringParseError> T>
	constexpr Value parse(Value::StringType const& str) {
		T parser;
		auto const result = parser.tryParse(str);
		if (result)
			return result.value();
		else result.error().value().raise();
	}

	/// @brief Parses bytes with a given parser.
	/// @tparam T Parser type.
	/// @param str Data to parse.
	/// @return Parsed data.
	/// @throw Error::FailedAction on parse failures.
	template <Type::Data::ByteParser<ByteParseError> T>
	constexpr Value parse(Value::ByteListType const& bytes) {
		T parser;
		auto const result = parser.tryParse(bytes);
		if (result)
			return result.value();
		else result.error().value().raise();
	}
}

CTL_EX_NAMESPACE_END

#endif