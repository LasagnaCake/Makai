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

		using CustomTypeParser = ResultType(LexerType&, FLOWParser&);

		constexpr static char const BINARY_IDENTIFIER			= '!';
		constexpr static char const ID_IDENTIFIER				= '#';
		constexpr static char const CUSTOM_TYPE_IDENTIFIER		= '$';
		constexpr static char const INTERNAL_TYPE_IDENTIFIER	= '@';

		/// @brief Tries to parse a FLOW string.
		/// @param str String to parse.
		/// @return Resulting value, or an error.
		ResultType tryParse(Value::StringType const& str) override {
			source = str;
			lexer.open(str.toString());
			if (!lexer.next()) return Value();
			auto const result = parseValue();
			lexer.next();
			if (!result) return result;
			if (lexer.next()) return error("Malformed value (extra unparsed data)!");
			//if (!lexer.finished()) return error("Malformed value (extra unparsed data)!");
			lexer.close();
			return result;
		}

		ResultType unknownTypeError() const {
			return error("Unknown/unsupported custom type!");
		}

		Function<CustomTypeParser> customTypeParser = [&] (auto&, auto&) {
			return unknownTypeError();
		};

	private:
		/// @brief Lexer token type.
		using TokenType = LexerType::Token::Type;

		ResultType parseNegativeNumber() {
			if (!lexer.next()) return error("Missing number value!");
			auto const token = lexer.current();
			switch (token.type) {
			case TokenType::LTS_TT_INTEGER:
				return Value(-token.value.get<ssize>());
			case TokenType::LTS_TT_REAL:
				return Value(-token.value.get<double>());
			default: return error("Value is not a negative number!");
			}
		}

		ResultType parseValue() {
			auto const token = lexer.current();
			switch (token.type) {
			case TokenType{'-'}:
				return parseNegativeNumber();
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
			case TokenType{BINARY_IDENTIFIER}:
				return parseBytes();
			case TokenType{ID_IDENTIFIER}:
				return parseIdentifier();
			case TokenType{CUSTOM_TYPE_IDENTIFIER}:
				return customTypeParser(lexer, *this);
			case TokenType::LTS_TT_IDENTIFIER: {
				auto const id = token.value.get<Value::StringType>();
				if (id == "null") return Value::null();
				else if (id == "true") return Value(true);
				else if (id == "false") return Value(false);
				else if (id == "nan") return Value(Value::NotANumber());
				else if (id == "undefined") return Value::undefined();
				return Value(id);
			}
			case TokenType{'}'}:
			case TokenType{']'}:
				return error("Unexpected closure!");
			default: return Value();
				//return error("Missing or invalid token!");
			}
			return Value();
		}
		
		ResultType parseIdentifier() {
			if (lexer.current().type != TokenType{'!'})
				return error("This is not an identifier!");
			if (!lexer.next()) return error("Missing identifier value!");
			if (lexer.current().type != TokenType{'['})
				return error("Expected '[' here!");
			As<uint64[Value::IdentifierType::SIZE]> id;
			if (!lexer.next()) return error("Missing identifier value!");
			for (usize i = 0; i < Value::IdentifierType::SIZE; ++i) {
				if (lexer.current().type != TokenType::LTS_TT_INTEGER)
					return error("Invalid identifier!");
				id[i] = lexer.current().value.getUnsigned();
				if (!lexer.next()) return error("Missing identifier value!");
				if (lexer.current().type == TokenType{'-'} && !lexer.next())
					return error("Missing identifier value!");
			}
			if (lexer.current().type != TokenType{']'})
				return error("Expected ']' here!");
			return Value(Value::IdentifierType::create(id));
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
				return error("String is not a valid FLOW array!");
			while (lexer.next()) {
				auto const token = lexer.current();
				if (token.type == TokenType{']'}) return result;
				switch (token.type) {
				case TokenType{'-'}:
				case TokenType::LTS_TT_INTEGER:
				case TokenType::LTS_TT_SINGLE_QUOTE_STRING:
				case TokenType::LTS_TT_DOUBLE_QUOTE_STRING:
				case TokenType::LTS_TT_REAL:
				case TokenType::LTS_TT_CHARACTER:
				case TokenType::LTS_TT_IDENTIFIER:
				case TokenType{BINARY_IDENTIFIER}:
				case TokenType{ID_IDENTIFIER}:
				case TokenType{'{'}:
				case TokenType{'['}:
				case TokenType{CUSTOM_TYPE_IDENTIFIER}: {
					auto v = parseValue();
					if (v) {
						auto const vv = v.value();
						result[result.size()] = vv;
						if (result[result.size()].isArray() && lexer.current().type == TokenType{']'})
							lexer.next();
					}
					else return v.error().value();
				} break;
				default: continue;
				}
				if (token.type == TokenType{']'}) return result;
			}
			if (lexer.current().type != TokenType{']'})
				return error("Missing closing bracket!");
			return result;
		}

		ResultType parseKeyValuePair() {
			Value result = Value::object();
			Value::StringType key;
			if (lexer.current().type == TokenType{'}'})
				return result;
			// Get key
			do {
				auto const token = lexer.current();
				if (lexer.current().type == TokenType{'}'})
					return result;
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
				case TokenType{BINARY_IDENTIFIER}:
				case TokenType{'{'}:
				case TokenType{'['}:
				case TokenType{'-'}:
				case TokenType{CUSTOM_TYPE_IDENTIFIER}:
					return error("Object key is not a string or identifier!");
				default: continue;
				}
				break;
			} while (lexer.next());
			lexer.next();
			if (lexer.current().type == TokenType{'}'})
				return error("Missing value for key \"" + key + "\"!");
			// Get value
			do {
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
				case TokenType::LTS_TT_IDENTIFIER:
				case TokenType{BINARY_IDENTIFIER}:
				case TokenType{'{'}:
				case TokenType{'['}:
				case TokenType{CUSTOM_TYPE_IDENTIFIER}: {
					auto const v = parseValue();
					if (v) {
						auto const vv = v.value();
						result[key] = vv;
						return result;
					}
					else return v.error().value();
				} break;
				default: continue;
				}
				break;
			} while (lexer.next());
			return result;
		}
		
		ResultType parseObject() {
			Value result = Value::object();
			if (lexer.current().type != TokenType{'{'})
				return error("String is not a valid FLOW object!");
			while (lexer.next()) {
				if (lexer.current().type == TokenType{'}'})
					return result;
				auto const v = parseKeyValuePair();
				if (v) {
					for (auto [k, v]: v.value().items())
						result[k] = v;
				} else return v.error().value();
			}
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