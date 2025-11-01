#ifndef MAKAILIB_PARSER_DATA_JSONPARSER_H
#define MAKAILIB_PARSER_DATA_JSONPARSER_H

#include "dataparser.hpp"

CTL_DIAGBLOCK_BEGIN
_Pragma("GCC diagnostic ignored \"-Wswitch\"")
/// @brief Data format parsers.
namespace Makai::Parser::Data {
	/// @brief JavaScript Object Notation (JSON) parser.
	struct JSONParser: IStringParser {
		/// @brief Lexer type.
		using LexerType = Lexer::CStyle::TokenStream;
		
		/// @brief Tries to parse a JSON string.
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
			case TokenType{'-'}:
				if (!lexer.next()) return error("Missing number value!");
				if (lexer.current().value.isInteger())
					return Value(-lexer.current().value.get<ssize>());
				else if (lexer.current().value.isReal())
					return Value(-lexer.current().value.get<double>());
				else return error("Value is not a negative number!");
			case TokenType::LTS_TT_INTEGER:
				return Value(static_cast<usize>(token.value.get<usize>()));
			case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
			case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
			case TokenType::LTS_TT_REAL:
				return token.value;
			case TokenType::LTS_TT_CHARACTER:
				return Value(toString(Cast::as<char>(token.value.get<ssize>())));
			case TokenType{'{'}:
				return parseObject();
			case TokenType{'['}:
				return parseArray();
			case TokenType::LTS_TT_IDENTIFIER: {
				auto const id = token.value.get<String>();
				if (id == "null") return Value::null();
				else if (id == "true")	return Value(true);
				else if (id == "false")	return Value(false);
				return error("Invalid/unsupported identifier!");
			}
			default:
				return error("Missing or invalid token [" + toString(enumcast(token.type)) + "]!");
			}
		}

		ResultType parseArray() {
			Value result = Value::array();
			if (lexer.current().type != TokenType{'['})
				return error("String is not a valid JSON array!");
			while (lexer.next()) {
				auto const token = lexer.current();
				switch (token.type) {
				case TokenType{'-'}:
					if (!lexer.next()) return error("Missing number value!");
					if (lexer.current().value.isInteger())
						result[result.size()] = Value(-lexer.current().value.get<ssize>());
					else if (lexer.current().value.isReal())
						result[result.size()] = Value(-lexer.current().value.get<double>());
					else return error("Value is not a negative number!");
				break;
				case TokenType::LTS_TT_INTEGER:
					result[result.size()] = Value(static_cast<usize>(token.value.get<usize>()));
				case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
				case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
				case TokenType::LTS_TT_REAL:
					return token.value;
				case TokenType::LTS_TT_CHARACTER:
					result[result.size()] = toString(Cast::as<char>(token.value.get<ssize>()));
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
				case TokenType::LTS_TT_IDENTIFIER: {
					auto const id = token.value.get<Value::StringType>();
					if (id == "null") result[result.size()] = Value::null();
					else if (id == "true") result[result.size()]	= true;
					else if (id == "false") result[result.size()]	= false;
					else return error("Invalid/unsupported identifier!");
				} break;
				case TokenType{','}:
					continue;
				default:
					return error("Missing or invalid token [" + toString(enumcast(token.type)) + "]!");
				}
				lexer.next();
				if (
					lexer.current().type != TokenType{','}
				&&	!(
						lexer.next()
					&&	lexer.current().type != TokenType{']'}
					)
				) return error("Malformed array or malformed element (missing separator comma)!");
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
				DEBUGLN(enumcast(token.type));
				if (token.type == TokenType{'}'})
					break;
				if (token.type == TokenType{':'}) continue;
				if (inValue) {
					switch (token.type) {
					case TokenType{'-'}:
						if (!lexer.next()) return error("Missing number value!");
						if (lexer.current().value.isInteger())
							result[key] = Value(-lexer.current().value.get<ssize>());
						else if (lexer.current().value.isReal())
							result[key] = Value(-lexer.current().value.get<double>());
						else return error("Value is not a negative number!");
					break;
					case TokenType::LTS_TT_INTEGER:
						result[key] = Value(static_cast<usize>(token.value.get<usize>()));
					case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
					case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
					case TokenType::LTS_TT_REAL:
						return token.value;
					break;
					case TokenType::LTS_TT_CHARACTER:
						result[key] = toString(Cast::as<char>(token.value.get<ssize>()));
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
					case TokenType::LTS_TT_IDENTIFIER: {
						auto const id = token.value.get<Value::StringType>();
						if (id == "null") result[key] = Value::null();
						else if (id == "true") result[key]	= true;
						else if (id == "false") result[key]	= false;
						else return error("Invalid/unsupported identifier!");
					} break;
					case TokenType{','}:
						inValue = false;
						continue;
					default:
						return error("Missing or invalid token [" + toString(enumcast(token.type)) + "]!");
					}
					lexer.next();
					DEBUGLN(enumcast(token.type));
					if (
						lexer.current().type != TokenType{','}
					&&	!(
							lexer.next()
						&&	lexer.current().type != TokenType{'}'}
						)
					) return error("Malformed object or malformed entry (missing separator comma)!");
					inValue = false;
					DEBUGLN(enumcast(token.type));
				} else {
					if (
						token.type == TokenType::LTS_TT_SINGLE_QUOTE_STRING
					||	token.type == TokenType::LTS_TT_DOUBLE_QUOTE_STRING
					) {
						key = token.value.get<Value::StringType>();
					} else if (token.type == TokenType::LTS_TT_CHARACTER) {
						key = (toString(Cast::as<char>(token.value.get<ssize>())));
					} else return error("Object key [" + toString(enumcast(token.type)) + "] is not a string!");
					lexer.next();
					DEBUGLN(enumcast(token.type));
					if (lexer.current().type != TokenType{':'})
						return error("Malformed object entry (separator colon)!");
					inValue = true;
					lexer.next();
					DEBUGLN(enumcast(token.type));
				}
				if (lexer.current().type == TokenType{'}'})
					break;
			}
			if (lexer.current().type != TokenType{'}'})
				return error("Missing closing curly bracket!");
			// lexer.next();
			return result;
		}
	
		ResultType error(String const& what) const {
			auto const loc = lexer.position();
			auto const lines = source.split('\n'); 
			return StringParseError{{loc.at, loc.line, loc.column+1}, what, lexer.tokenText()};
		}
		
		/// @brief String source.
		Value::StringType	source;
		/// @brief Underlying lexer.
		LexerType 			lexer;
	};
}
CTL_DIAGBLOCK_END

#endif