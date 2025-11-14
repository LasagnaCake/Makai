#ifndef MAKAILIB_PARSER_DATA_DATAPARSER_H
#define MAKAILIB_PARSER_DATA_DATAPARSER_H

#include "../../lexer/lexer.hpp"

/// @brief Data format parsers.
namespace Makai::Parser::Data {
	using namespace CTL::Ex::Data;
	/// @brief String parser interface.
	struct IStringParser {
		/// @brief Result type.
		using ResultType = Result<Value, StringParseError>;

		/// @brief Virtual destructor.
		constexpr virtual ~IStringParser() {}

		/// @brief Tries to parse a string. Must be implemented.
		constexpr virtual ResultType tryParse(Value::StringType const& str) = 0;	
	};

	/// @brief Byte parser interface.
	struct IByteParser {
		/// @brief Result type.
		using ResultType = Result<Value, ByteParseError>;

		/// @brief Virtual destructor.
		constexpr virtual ~IByteParser() {}

		/// @brief Tries to parse a series of bytes. Must be implemented.
		constexpr virtual ResultType tryParse(Value::ByteListType const& str) = 0;	
	};
}

#endif