#ifndef MAKAILIB_PARSER_DATA_JSONPARSER_H
#define MAKAILIB_PARSER_DATA_JSONPARSER_H

#include "dataparser.hpp"

/// @brief Data format parsers.
namespace Makai::Parser::Data {
	/// @brief JavaScript Object Notation (JSON) parser.
	struct JSONParser: IStringParser {
		/// @brief Lexer type.
		using LexerType = Lexer::CStyle::TokenStream;
		
		/// @brief Tries to parse a JSON string.
		/// @param str String to parse.
		/// @return Resulting value, or an error.	
		constexpr ResultType tryParse(Value::StringType const& str) override {
			source = str;
			lexer.open(String(str));
			auto const result = parse(str);
			lexer.close();
			return result;
		}

	private:
		/// @brief Lexer token type.
		using TokenType = LexerType::Token::Type;
		/// @brief Lexer token value type.
		using ValueType = LexerType::Token::Value::Type;

		ResultType parse(Value::StringType const& str) {
			if (!lexer.next()) return Value();
			switch (token.type) {
			case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
			case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
				return Value(token.value.template get<ValueType::LTS_TVT_STRING>().value());
			case TokenType::LTS_TT_INTEGER:
				return Value(token.value.template get<ValueType::LTS_TVT_INTEGER>().value());
			case TokenType::LTS_TT_REAL:
				return Value(token.value.template get<ValueType::LTS_TVT_REAL>().value());
			case TokenType{'{'}:
				return parseObject();
			case TokenType{'['}:
				return parseArray();
			case TokenType::LTS_TT_IDENTIFIER: {
				auto const id = token.value.template get<ValueType::LTS_TVT_STRING>().value();
				if (id == "null") return Value::null();
				return error("Invalid/unsupported identifier!");
			}
			default:
				return error("Missing or invalid token!");
			}
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
					result[result.size()] = Value(token.value.template get<ValueType::LTS_TVT_STRING>().value());
				break;
				case TokenType::LTS_TT_INTEGER:
					result[result.size()] = Value(token.value.template get<ValueType::LTS_TVT_INTEGER>().value());
				break;
				case TokenType::LTS_TT_REAL:
					result[result.size()] = Value(token.value.template get<ValueType::LTS_TVT_REAL>().value());
				break;
				case TokenType{'{'}: {
					auto const obj = parseObject();
					if (obj)
						result[result.size()] = obj.value();
					else return obj.error();
				} break;
				case TokenType{'['}: {
					auto const obj = parseArray();
					if (obj)
						result[result.size()] = obj.value();
					else return obj.error();
				} break;
				case TokenType::LTS_TT_IDENTIFIER: {
					auto const id = token.value.template get<ValueType::LTS_TVT_STRING>().value();
					if (id == "null") result[result.size()] = Value::null();
					else return error("Invalid/unsupported identifier!");
				} break;
				default:
					return error("Missing or invalid token!");
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
				if (token.type == '}')
					break;
				if (token.type == TokenType{':'}) continue;
				if (inValue) {
					switch (token.type) {
					case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
					case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
						result[key] = Value(token.value.template get<ValueType::LTS_TVT_STRING>().value());
					break;
					case TokenType::LTS_TT_INTEGER:
						result[key] = Value(token.value.template get<ValueType::LTS_TVT_INTEGER>().value());
					break;
					case TokenType::LTS_TT_REAL:
						result[key] = Value(token.value.template get<ValueType::LTS_TVT_REAL>().value());
					break;
					case TokenType{'{'}: {
						auto const obj = parseObject();
						if (obj)
							result[key] = obj.value();
						else return obj.error();
					} break;
					case TokenType{'['}: {
						auto const obj = parseArray();
						if (obj)
							result[key] = obj.value();
						else return obj.error();
					} break;
					case TokenType::LTS_TT_IDENTIFIER: {
						auto const id = token.value.template get<ValueType::LTS_TVT_STRING>().value();
						if (id == "null") result[rkey] = Value::null();
						else return error("Invalid/unsupported identifier!");
					} break;
					default:
						return error("Missing or invalid token!");
					}
					lexer.next();
					if (
						lexer.current().type != TokenType{','}
					&&	!(
							lexer.next()
						&&	lexer.current().type != TokenType{'}'}
						)
					) return error("Malformed object or malformed entry (missing separator comma)!");
					inValue = false;
					break;
				} else {
					if (
						token.type == TokenType::LTS_TT_SINGLE_QUOTE_STRING
					||	token.type == TokenType::LTS_TT_DOUBLE_QUOTE_STRING
					) {
						key = token.template get<ValueType::LTS_TVT_STRING>().value();
						lexer.next();
						if (lexer.current().type != TokenType{':'})
							return error("Malformed object entry (separator colon)!");
						inValue = true;
					} else return error("Object key is not a string!");
				}
				if (lexer.current().type == TokenType{'}'})
					break;
			}
			if (lexer.current().type != TokenType{'}'})
				return error("Missing closing curly bracket!");
			// lexer.next();
			return result;
		}
	
		constexpr ParseError error(String const& what) const {
			auto const loc = lexer.position();
			auto const lines = source.split('\n'); 
			return ParseError{{loc.at, loc.line, loc.column}, what, lines[loc.line % lines.size()].substring(loc.column, 80)};
		}
		
		/// @brief String source.
		Value::StringType	source;
		/// @brief Underlying lexer.
		LexerType 			lexer;
	}
}

#endif