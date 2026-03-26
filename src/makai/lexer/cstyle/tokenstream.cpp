#include "tokenstream.hpp"

using namespace Makai;
using namespace Makai::Lexer;
using namespace Makai::Lexer::CStyle;
using enum TokenStream::Token::Type;

struct TokenStream::Lexer {
	UTF8String const source;
	UTF8String stream = source;

	usize total		= source.size();
	usize curLine	= 0;
	usize curCol	= 0;

	Lexer(UTF8String const& src): source(src), stream(src), total(src.size()) {}

	bool empty() const {
		return stream.empty();
	}

	UTF::U8Char next() {
		if (empty()) return {};
		stream.popBack();
		if (empty()) return {};
		++curCol;
		if (stream.back() == UTF::U8Char{'\n'}) {
			++curLine;
			curCol = 0;
		}
		return stream.back();
	}

	UTF::U8Char peek(usize const ahead = 0) const {
		if ((ahead + 2) < stream.size())
			return stream[- ahead - 2];
		return {};
	}

	UTF::U8Char now() const {
		if (empty()) return {};
		return stream.back();
	}
};

static bool isNumberChar(UTF::U8Char const ch) {
	if (ch >= UTF::U8Char{'0'} && ch <= UTF::U8Char{'9'}) return true;
	return false;
}

static bool isWordChar(UTF::U8Char const ch) {
	if (ch >= UTF::U8Char{'A'} && ch <= UTF::U8Char{'Z'}) return true;
	if (ch >= UTF::U8Char{'a'} && ch <= UTF::U8Char{'z'}) return true;
	if (ch == UTF::U8Char{'_'}) return true;
	return false;
}

static bool isIdentifierChar(UTF::U8Char const ch) {
	return isNumberChar(ch) || isWordChar(ch);
}

static bool isOtherNumberChar(UTF::U8Char const ch) {
	if (ch == UTF::U8Char{'.'}) return true;
	return false;
}

static bool isSpaceChar(UTF::U8Char const ch) {
	if (ch == UTF::U8Char{'\n'}) return true;
	if (ch == UTF::U8Char{'\v'}) return true;
	if (ch == UTF::U8Char{'\t'}) return true;
	if (ch == UTF::U8Char{'\r'}) return true;
	if (ch == UTF::U8Char{' '}) return true;
	if (ch == UTF::U8Char{}) return true;
	return false;
}

static UTF::U8Char unescape(UTF::U8Char const ch) {
	if (ch == UTF::U8Char{'n'}) return {'\n'};
	if (ch == UTF::U8Char{'t'}) return {'\t'};
	if (ch == UTF::U8Char{'v'}) return {'\v'};
	if (ch == UTF::U8Char{'r'}) return {'\r'};
	if (ch == UTF::U8Char{'b'}) return {'\b'};
	return ch;
}

static UTF8String parseString(TokenStream::Lexer& lexer, UTF::U8Char const delim = {'\"'}) {
	UTF8String result;
	while (lexer.now() != delim) {
		if (lexer.now() == UTF::U8Char{'\\'})
			result.pushBack(unescape(lexer.next()));
		else result.pushBack(lexer.now());
		lexer.next();
		if (lexer.now() == delim) break;
	}
	DEBUGLN("String: ", result);
	return result;
}

static UTF8String parseID(TokenStream::Lexer& lexer) {
	UTF8String result;
	while (isIdentifierChar(lexer.now())) {
		result.pushBack(lexer.now());
		lexer.next();
	}
	return result;
}

static UTF8String parseNumber(TokenStream::Lexer& lexer) {
	UTF8String result;
	while (isIdentifierChar(lexer.now()) || isOtherNumberChar(lexer.now())) {
		result.pushBack(lexer.now());
		lexer.next();
	}
	return result;
}

constexpr UTF::U8Char closingQuote(UTF::U8Char const op) {
	switch (op.value()) {
		case (UTF::U8Char{'\''}.value()): return {'\''};
		case (UTF::U8Char{'\"'}.value()): return {'\"'};
		case (UTF::U8Char{'`'}.value()): return {'`'};
		case (UTF::U8Char{"«"}.value()): return {"»"};
		case (UTF::U8Char{"「"}.value()): return {"」"};
		case (UTF::U8Char{"『"}.value()): return {"』"};
		case (UTF::U8Char{"‹"}.value()): return {"›"};
	}
	return {};
}

constexpr TokenStream::Token::Type stringType(UTF::U8Char const op) {
	switch (op) {
		case (UTF::U8Char{'\''}): return LTS_TT_SINGLE_QUOTE_STRING;
		case (UTF::U8Char{'\"'}): return LTS_TT_DOUBLE_QUOTE_STRING;
		case (UTF::U8Char{'`'}): return LTS_TT_BACKTICK_STRING;
		case (UTF::U8Char{"‹"}): return LTS_TT_FR_DOUBLE_QUOTE_STRING;
		case (UTF::U8Char{"«"}): return LTS_TT_FR_SINGLE_QUOTE_STRING;
		case (UTF::U8Char{"「"}): return LTS_TT_JP_SINGLE_QUOTE_STRING;
		case (UTF::U8Char{"『"}): return LTS_TT_JP_DOUBLE_QUOTE_STRING;
	}
	return LTS_TT_INVALID;
}

static void parseOperator(TokenStream::Lexer& lexer, TokenStream::Token& tok) {
	DEBUGLN("Operator time!");
	tok.type = TokenStream::Token::Type{lexer.now().value()};
	tok.text.pushBack(lexer.now());
	if (lexer.now() == UTF::U8Char{'='}) {
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_COMPARE_EQUALS;
		} else if (lexer.peek() == UTF::U8Char{'>'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIG_ARROW;
		}
	} else if (lexer.now() == UTF::U8Char{'|'}) {
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIT_OR_ASSIGN;
		} else if (lexer.peek() == UTF::U8Char{'|'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LOGIC_OR;
		} else if (lexer.peek() == UTF::U8Char{'>'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_STREAM_EXTRACT;
		}
	} else if (lexer.now() == UTF::U8Char{'<'}) {
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			if (lexer.peek() == UTF::U8Char{'>'}) {
				tok.text.pushBack(lexer.next());
				tok.type = LTS_TT_ORDER;
			} else tok.type = LTS_TT_COMPARE_LESS_EQUALS;
		}
		if (lexer.peek() == UTF::U8Char{'|'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_STREAM_INSERT;
		}
		if (lexer.peek() == UTF::U8Char{'<'}) {
			tok.text.pushBack(lexer.next());
			if (lexer.peek() == UTF::U8Char{'='}) {
				tok.text.pushBack(lexer.next());
				tok.type = LTS_TT_BIT_SHIFT_LEFT_ASSIGN;
			} else tok.type = LTS_TT_BIT_SHIFT_LEFT;
		}
	} else if (lexer.now() == UTF::U8Char{'-'}) {
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_SUB_ASSIGN;
		} else if (lexer.peek() == UTF::U8Char{'-'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_DECREMENT;
		} else if (lexer.peek() == UTF::U8Char{'>'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LITTLE_ARROW;
		}
	} else if (lexer.now() == UTF::U8Char{':'}) {
		if (lexer.peek() == UTF::U8Char{':'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_NAMESPACE_RESOLVE;
		} else if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_DECLARE;
		}
	} else if (lexer.now() == UTF::U8Char{'+'}) {
		if (lexer.peek() == UTF::U8Char{'+'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_INCREMENT;
		} else if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_ADD_ASSIGN;
		}
	} else if (lexer.now() == UTF::U8Char{'*'}) {
		if (lexer.peek() == UTF::U8Char{'.'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_PTR_ACCESS;
		} else if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_MUL_ASSIGN;
		}
	} else if (lexer.now() == UTF::U8Char{'/'} && lexer.peek() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_DIV_ASSIGN;
	} else if (lexer.now() == UTF::U8Char{'%'} && lexer.peek() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_MOD_ASSIGN;
	} else if (lexer.now() == UTF::U8Char{'^'}) {
		if (lexer.peek() == UTF::U8Char{'^'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LOGIC_XOR;
		} else if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIT_XOR_ASSIGN;
		} else if (lexer.peek() == UTF::U8Char{'.'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_DEREF_ACCESS;
		}
	} else if (lexer.now() == UTF::U8Char{'&'}) {
		if (lexer.peek() == UTF::U8Char{'&'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LOGIC_AND;
		} else if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIT_AND_ASSIGN;
		}
	} else if (lexer.now() == UTF::U8Char{'|'}) {
		if (lexer.peek() == UTF::U8Char{'|'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LOGIC_OR;
		} else if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIT_OR_ASSIGN;
		}
	} else if (lexer.now() == UTF::U8Char{'!'} && lexer.now() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_COMPARE_NOT_EQUALS;
	} else if (lexer.now() == UTF::U8Char{'~'} && lexer.now() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_BIT_NOT_ASSIGN;
	} else if (lexer.now() == UTF::U8Char{'>'} && lexer.now() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_COMPARE_GREATER_EQUALS;
	} else if (lexer.now() == UTF::U8Char{'?'}) {
		if (lexer.peek() == UTF::U8Char{'.'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_NULL_ACCESS;
		} else if (lexer.peek() == UTF::U8Char{'?'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_NULL_DECAY;
		} else if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_NULL_ASSIGN;
		}
	} else if (lexer.now() == UTF::U8Char{'.'}) {
		if (lexer.peek() == UTF::U8Char{'.'}) {
			tok.text.pushBack(lexer.next());
			if (lexer.peek() == UTF::U8Char{'.'}) {
				tok.text.pushBack(lexer.next());
				tok.type = LTS_TT_ELLIPSIS;
			}// else tok.type = LTS_TT_RANGE;
		}
	}
	lexer.next();
}

static void parseBlockComment(TokenStream::Lexer& lexer) {
	while (!(lexer.now() == UTF::U8Char{'*'} && lexer.peek() == UTF::U8Char{'/'}))
		lexer.next();
}

static void parseLineComment(TokenStream::Lexer& lexer) {
	while (lexer.now() != UTF::U8Char{'\n'})
		lexer.next();
}

bool TokenStream::next() {
	if (!lexer || isFinished) return false;
	while (isSpaceChar(lexer->now()) && !lexer->empty())
		lexer->next();
	if (lexer->empty()) {
		isFinished = true;
		return false;
	}
	UTF::U8String lexeme;
	curToken = {.at = position()};
	DEBUGLN("Char: ", (char)lexer->now().value(), ", next: ", (char)lexer->peek().value());
	if (isNumberChar(lexer->now()) || ((lexer->now() == UTF::U8Char{'.'}) && !isWordChar(lexer->peek()))) {
		lexeme = parseNumber(*lexer);
		try {
			if (lexeme.find({'.'}) != -1) {
				curToken.type = Token::Type::LTS_TT_INTEGER;
				curToken.value = toDouble(lexeme.toString());
			}
			if (lexeme.find({'.'}) != -1) {
				curToken.type = Token::Type::LTS_TT_REAL;
				curToken.value = toInt64(lexeme.toString());
			}
		} catch (...) {
			err = Error{curToken.at, lexeme};
			isFinished = true;
		}
	}
	else if (lexer->now() == UTF::U8Char{'/'} && lexer->peek() == UTF::U8Char{'*'})
		parseBlockComment(*lexer);
	else if (lexer->now() == UTF::U8Char{'/'} && lexer->peek() == UTF::U8Char{'/'})
		parseLineComment(*lexer);
	else if (isWordChar(lexer->now())) {
		lexeme = parseID(*lexer);
		curToken.text = lexeme;
		curToken.value = curToken.text.toString();
		curToken.type = LTS_TT_IDENTIFIER;
	}
	else if (closingQuote(lexer->now()) != UTF::U8Char{}) {
		auto const quot = closingQuote(lexer->now());
		curToken.type = stringType(lexer->now());
		lexer->next();
		lexeme = parseString(*lexer, quot);
		curToken.text	= lexeme;
		curToken.value	= curToken.text.toString();
	}
	else parseOperator(*lexer, curToken);
	if (lexeme.size())
		curToken.text = lexeme;
	if (lexer->empty())
		isFinished = true;
	DEBUGLN("Type: ", Token::asName(curToken.type));
	DEBUGLN("Text: ", curToken.text.toString());
	return !isFinished;
}

Makai::UTF8String TokenStream::tokenText() const {
	if (!lexer) return "";
	return curToken.text;
}

TokenStream::Position TokenStream::position() const {
	if (!lexer) return {
		Limit::MAX<usize>,
		Limit::MAX<usize>,
		Limit::MAX<usize>
	};

	return {
		location(),
		lexer->curLine,
		lexer->curCol
	};
}

usize TokenStream::location() const {
	if (!lexer) return -1;
	return lexer->total - lexer->source.size();
}

void TokenStream::assertOK() const {
	if (err)
		throw TokenStream::InvalidToken(
			toString(
				"Invalid token \"", err.value().token, "\"!"
				"\nAt line [", err.value().where.line,"], column [", err.value().where.column,"]"
			),
			CTL_CPP_PRETTY_SOURCE
		);
}

TokenStream::TokenStream(UTF8String const& source)	{open(source);	}
TokenStream::TokenStream()							{				}
TokenStream::~TokenStream()							{close();		}

CStyle::TokenStream& TokenStream::open(UTF8String const& source) {
	if (lexer) return *this;
	lexer.bind(new Lexer{source.reversed()});
	DEBUGLN("Source size: ", lexer->source.size());
	err = nullptr;
	isFinished = false;
	return *this;
}

CStyle::TokenStream& TokenStream::close() {
	if (!lexer) return *this;
	lexer.unbind();
	isFinished = true;
	return *this;
}

bool TokenStream::finished() const {
	return isFinished;
}

Makai::Result<TokenStream::TokenList, TokenStream::Error>
Makai::Lexer::CStyle::tokenize(Makai::UTF8String const& source) {
	TokenStream::TokenList result;
	TokenStream stream{source};
	while (stream.next())
		result.pushBack(stream.current());
	if (!stream.ok()) return *stream.error();
	return result;
}
