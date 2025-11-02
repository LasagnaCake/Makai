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
			if (!lexer.next()) return Value();
			auto const result = parseValue();
			lexer.close();
			return result;
		}

	private:
		/// @brief Lexer token type.
		using TokenType = LexerType::Token::Type;

		ResultType parseValue() {
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
			return Value();
		}

		ResultType parseBytes() {
			if (lexer.current().type != TokenType{'!'})
				return error("String is not a valid byte string!");
			if (!lexer.next()) return error("Missing byte string format specifier!");
			if (!(
				lexer.current().type == TokenType::LTS_TT_INTEGER
			||	lexer.current().type == TokenType::LTS_TT_REAL
			))
				return error("Invalid byte string format specifier!");
			usize const base = lexer.current().value;
			if (!lexer.next()) return error("Missing byte string contents!");
			if (!(
				lexer.current().type == TokenType::LTS_TT_SINGLE_QUOTE_STRING
			||	lexer.current().type == TokenType::LTS_TT_DOUBLE_QUOTE_STRING
			)) return error("Invalid byte string contents!");
			String const str = lexer.current().value;
			switch (base) {
				case 2:		return Value(Convert::fromBase<Convert::Base::CB_BASE2>(str));
				case 4:		return Value(Convert::fromBase<Convert::Base::CB_BASE4>(str));
				case 8:		return Value(Convert::fromBase<Convert::Base::CB_BASE8>(str));
				case 16:	return Value(Convert::fromBase<Convert::Base::CB_BASE16>(str));
				case 32:	return Value(Convert::fromBase<Convert::Base::CB_BASE32>(str));
				case 64:	return Value(Convert::fromBase<Convert::Base::CB_BASE64>(str));
				default:	return error("Invalid string format specifier!");
			}
		}

		ResultType parseArray() {
			Value result = Value::array();
			if (lexer.current().type != TokenType{'['})
				return error("String is not a valid JSON array!");
			while (lexer.next()) {
				auto const token = lexer.current();
				if (token.type == TokenType{']'}) break;
				switch (token.type) {
				case TokenType{']'}: break;
				case TokenType{'-'}:
				case TokenType::LTS_TT_INTEGER:
				case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
				case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
				case TokenType::LTS_TT_REAL:
				case TokenType::LTS_TT_CHARACTER:
				case TokenType{'!'}:
				case TokenType{'{'}:
				case TokenType{'['}: {
					auto v = parseValue();
					if (v) result[result.size()] = v.value();
					else return v.error().value();
				} break;
				default: continue;
				}
				if (lexer.current().type == TokenType{']'}) break;
			}
			if (lexer.current().type != TokenType{']'})
				return error("Missing closing bracket!");
			return result;
		}

		ResultType parseObject() {
			Value result = Value::object();
			if (lexer.current().type != TokenType{'{'})
				return error("String is not a valid JSON object!");
			DEBUGLN("Object");
			do {
				Value::StringType key;
				if (lexer.current().type == TokenType{'}'})
					break;
				// Get key
				while (lexer.next()) {
					auto const token = lexer.current();
					switch (token.type) {
					case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
					case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
					case TokenType::LTS_TT_IDENTIFIER:
						key = token.value.get<Value::StringType>();
					break;
					case TokenType::LTS_TT_CHARACTER:
						key = toString(Cast::as<char>(token.value.get<ssize>()));
					break;
					case TokenType::LTS_TT_INTEGER:
					case TokenType::LTS_TT_REAL:
					case TokenType{'!'}:
					case TokenType{'{'}:
					case TokenType{'['}:
					case TokenType{'-'}:
						return error("Object key is not a string or identifier!");
					default: continue;
					}
					break;
				}
				// Get value
				while (lexer.next()) {
					auto const token = lexer.current();
					if (token.type == TokenType{'}'})
						return error("Missing value for key \"" + key + "\"!");
					switch (token.type) {
					case TokenType{'-'}:
					case TokenType::LTS_TT_INTEGER:
					case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
					case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
					case TokenType::LTS_TT_REAL:
					case TokenType::LTS_TT_CHARACTER:
					case TokenType{'!'}:
					case TokenType{'{'}:
					case TokenType{'['}: {
						auto v = parseValue();
						if (v) result[key] = v.value();
						else return v.error().value();
					} break;
					default: continue;
					}
					break;
				}
				if (lexer.current().type == TokenType{'}'})
					break;
			} while (!lexer.finished());
			if (lexer.current().type != TokenType{'}'})
				return error("Missing closing curly bracket!");
			return result;
		}
	
		StringParseError error(String const& what) const {
			auto const loc = lexer.position();
			return StringParseError{{loc.at, loc.line, loc.column+1}, what, source.substring(loc.at).split('\n').front().substring(0, 20)};
		}
		
		/// @brief String source.
		Value::StringType	source;
		/// @brief Underlying lexer.
		LexerType 			lexer;
	};
}
CTL_DIAGBLOCK_END

#endif