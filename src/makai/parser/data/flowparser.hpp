#ifndef MAKAILIB_PARSER_DATA_FLOWPARSER_H
#define MAKAILIB_PARSER_DATA_FLOWPARSER_H

#include "dataparser.hpp"

CTL_DIAGBLOCK_BEGIN
_Pragma("GCC diagnostic ignored \"-Wswitch\"")
/// @brief Data format parsers.
namespace Makai::Parser::Data {
	/// @brief Fast Lazy Object Writing (FLOW) parser.
	struct FLOWParser: IStringParser {
		/// @brief Lexer type.
		using LexerType = Lexer::CStyle::TokenStream;

		/// @brief Tries to parse a FLOW string.
		/// @param str String to parse.
		/// @return Resulting value, or an error.
		ResultType tryParse(Value::StringType const& str) override {
			source = str;
			lexer.open(str.toString());
			auto const result = parse(str);
			lexer.close();
			return result;
		}

	private:
		/// @brief Lexer token type.
		using TokenType = LexerType::Token::Type;

		ResultType parse(Value::StringType const& str) {
			if (!lexer.next()) return Value();
			auto const token = lexer.current();
			switch (token.type) {
			case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
			case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
			case TokenType::LTS_TT_INTEGER:
			case TokenType::LTS_TT_REAL:
				return token.value;
			case TokenType{'{'}:
				return parseObject();
			case TokenType{'['}:
				return parseArray();
			case TokenType{'!'}:
				return parseBytes();
			case TokenType::LTS_TT_IDENTIFIER: {
				auto const id = token.value.get<Value::StringType>();
				if (id == "null") return Value::null();
				else if (id == "true") return Value(true);
				else if (id == "false") return Value(false);
				return Value(id);
			}
			default: return Value();
				//return error("Missing or invalid token!");
			}
		}

		ResultType parseBytes() {
			if (lexer.current().type != TokenType{'!'})
				return error("String is not a valid byte string!");
			if (!lexer.next() || lexer.current().type != TokenType::LTS_TT_INTEGER)
				return error("Missing/Invalid byte string format specifier!");
			usize const base = lexer.current().value;
			if (!lexer.next() || !(
				lexer.current().type == TokenType::LTS_TT_SINGLE_QUOTE_STRING
			||	lexer.current().type == TokenType::LTS_TT_DOUBLE_QUOTE_STRING
			)) return error("Missing/Invalid byte string contents!");
			String const str = lexer.current().value;
			usize stride = 0;
			switch (base) {
				case 2:		stride = 8; break;
				case 4:		stride = 4; break;
				case 8:		stride = 3; break;
				case 16:	stride = 2; break;
				case 32:	stride = 2; break;
				default: return error("Invalid string format specifier!");
			}
			if (str.size() % stride != 0) return error("String size is not a proper multiple for the given format specifier!");
			usize start = 0;
			auto result = Value::ByteListType();
			while (start < str.size()) {
				result.pushBack(String::toNumber<byte>(str.substring(start, start + stride), base));
				start += stride;
			}
			return Value(result);
		}

		ResultType parseArray() {
			Value result = Value::array();
			if (lexer.current().type != TokenType{'['})
				return error("String is not a valid JSON array!");
			while (lexer.next()) {
				auto const token = lexer.current();
				switch (token.type) {
				case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
				case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
				case TokenType::LTS_TT_INTEGER:
				case TokenType::LTS_TT_REAL:
					result[result.size()] = token.value;
				break;
				case TokenType{'{'}: {
					auto const obj = parseObject();
					if (obj)
						result[result.size()] = obj.value();
					else return obj.error().orElse({.what = "Unknown object error!"});
				} break;
				case TokenType{'['}: {
					auto const obj = parseArray();
					if (obj)
						result[result.size()] = obj.value();
					else return obj.error().orElse({.what = "Unknown array error!"});
				} break;
				case TokenType{'!'}: {
					auto const obj = parseBytes();
					if (obj)
						result[result.size()] = obj.value();
					else return obj.error().orElse({.what = "Unknown byte string error!"});
				} break;
				case TokenType::LTS_TT_IDENTIFIER: {
					auto const id = token.value.get<Value::StringType>();
					if (id == "null") result[result.size()] = Value::null();
					else if (id == "true") result[result.size()] = true;
					else if (id == "false") result[result.size()] = false;
					else result[result.size()] = Value::StringType(id);
				} break;
				default:
					continue;
					//return error("Missing or invalid token!");
				}
			}
			if (lexer.current().type != TokenType{']'})
				return error("Missing closing bracket!");
			// lexer.next();
			return result;
		}

		ResultType parseObject() {
			Value result = Value::object();
			Value::StringType key;
			if (lexer.current().type != TokenType{'{'})
				return error("String is not a valid JSON object!");
			bool inValue = false;
			while (lexer.next()) {
				auto const token = lexer.current();
				if (token.type == TokenType{'}'})
					break;
				if (token.type == TokenType{':'}) continue;
				if (inValue) {
					switch (token.type) {
					case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
					case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
					case TokenType::LTS_TT_INTEGER:
					case TokenType::LTS_TT_REAL:
						result[key] = token.value;
					break;
					case TokenType{'{'}: {
						auto const obj = parseObject();
						if (obj)
							result[key] = obj.value();
						else return obj.error().orElse({.what = "Unknown object error!"});
					} break;
					case TokenType{'['}: {
						auto const obj = parseArray();
						if (obj)
							result[key] = obj.value();
						else return obj.error().orElse({.what = "Unknown array error!"});
					} break;
					case TokenType{'!'}: {
						auto const obj = parseBytes();
						if (obj)
							result[result.size()] = obj.value();
						else return obj.error().orElse({.what = "Unknown byte string error!"});
					} break;
					case TokenType::LTS_TT_IDENTIFIER: {
						auto const id = token.value.get<Value::StringType>();
						if (id == "null") result[key] = Value::null();
						else if (id == "true") result[key]	= true;
						else if (id == "false") result[key]	= false;
						else result[key] = Value::StringType(id);
					} break;
					default:
						continue;
						//return error("Missing or invalid token!");
					}
					inValue = false;
					break;
				} else {
					if (
						token.type == TokenType::LTS_TT_SINGLE_QUOTE_STRING
					||	token.type == TokenType::LTS_TT_DOUBLE_QUOTE_STRING
					||	token.type == TokenType::LTS_TT_IDENTIFIER
					) {
						key = token.value.get<Value::StringType>();
						lexer.next();
						inValue = true;
					} else if (
						token.type == TokenType::LTS_TT_INTEGER
					||	token.type == TokenType::LTS_TT_REAL
					) return error("Object key is not a string or identifier!");
					else continue;
				}
				if (lexer.current().type == TokenType{'}'})
					break;
			}
			if (lexer.current().type != TokenType{'}'})
				return error("Missing closing curly bracket!");
			// lexer.next();
			return result;
		}
	
		StringParseError error(String const& what) const {
			auto const loc = lexer.position();
			auto const lines = source.split('\n'); 
			return StringParseError{{loc.at, loc.line, loc.column}, what, lines[loc.line % lines.size()].substring(loc.column, 80)};
		}
		
		/// @brief String source.
		Value::StringType	source;
		/// @brief Underlying lexer.
		LexerType 			lexer;
	};
}
CTL_DIAGBLOCK_END

#endif