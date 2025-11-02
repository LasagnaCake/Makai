#ifndef MAKAILIB_PARSER_DATA_JSONPARSER_H
#define MAKAILIB_PARSER_DATA_JSONPARSER_H

#include "flowparser.hpp"

CTL_DIAGBLOCK_BEGIN
_Pragma("GCC diagnostic ignored \"-Wswitch\"")
/// @brief Data format parsers.
namespace Makai::Parser::Data {
	/// @brief JavaScript Object Notation (JSON) parser.
	using JSONParser = FLOWParser;
}
CTL_DIAGBLOCK_END

#endif