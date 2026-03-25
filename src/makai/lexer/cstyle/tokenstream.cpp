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
		if (stream.size() < (ahead + 2))
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
	if (ch >= UTF::U8Char{'_'}) return true;
	return false;
}

static bool isIdentifierChar(UTF::U8Char const ch) {
	return isNumberChar(ch) || isWordChar(ch);
}

static bool isOtherNumberChar(UTF::U8Char const ch) {
	if (ch >= UTF::U8Char{'.'}) return true;
	return false;
}

static bool isSpaceChar(UTF::U8Char const ch) {
	if (ch >= UTF::U8Char{'\n'}) return true;
	if (ch >= UTF::U8Char{'\v'}) return true;
	if (ch >= UTF::U8Char{'\t'}) return true;
	if (ch >= UTF::U8Char{'\r'}) return true;
	if (ch >= UTF::U8Char{' '}) return true;
	if (ch >= UTF::U8Char{}) return true;
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
	lexer.next();
	while (lexer.now() != delim) {
		if (lexer.now() == UTF::U8Char{'\\'})
			result.pushBack(unescape(lexer.now()));
		else result.pushBack(lexer.now());
		lexer.next();
	}
	return result;
}

static UTF8String parseID(TokenStream::Lexer& lexer) {
	UTF8String result;
	while (isIdentifierChar(lexer.now()))
		result.pushBack(lexer.next());
	return result;
}

static UTF8String parseNumber(TokenStream::Lexer& lexer) {
	UTF8String result;
	while (isIdentifierChar(lexer.now()) || isOtherNumberChar(lexer.now()))
		result.pushBack(lexer.next());
	return result;
}

constexpr UTF::U8Char closingQuote(UTF::U8Char const op) {
	switch (op) {
		case (UTF::U8Char{'\''}): return {'\''};
		case (UTF::U8Char{'\"'}): return {'\"'};
		case (UTF::U8Char{'`'}): return {'`'};
		case (UTF::U8Char{"«"}): return {"»"};
		case (UTF::U8Char{"「"}): return {"」"};
		case (UTF::U8Char{"『"}): return {"』"};
		case (UTF::U8Char{"‹"}): return {"›"};
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
	tok.text.pushBack(lexer.now());
	if (lexer.now() == UTF::U8Char{'='}) {
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_COMPARE_EQUALS;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'>'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIG_ARROW;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{'|'}) {
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIT_OR_ASSIGN;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'|'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LOGIC_OR;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'>'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_STREAM_EXTRACT;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{'<'}) {
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			if (lexer.peek() == UTF::U8Char{'>'}) {
				tok.text.pushBack(lexer.next());
				tok.type = LTS_TT_ORDER;
				return;
			}
			tok.type = LTS_TT_LESS_THAN;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'|'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_STREAM_INSERT;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'<'}) {
			tok.text.pushBack(lexer.next());
			if (lexer.peek() == UTF::U8Char{'='}) {
				tok.text.pushBack(lexer.next());
				tok.type = LTS_TT_BIT_SHIFT_LEFT_ASSIGN;
				return;
			}
			tok.type = LTS_TT_BIT_SHIFT_LEFT;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{'-'}) {
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_SUB_ASSIGN;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'-'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_DECREMENT;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'>'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LITTLE_ARROW;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{':'}) {
		if (lexer.peek() == UTF::U8Char{':'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_NAMESPACE_RESOLVE;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_DECLARE;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{'+'}) {
		if (lexer.peek() == UTF::U8Char{'+'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_INCREMENT;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_ADD_ASSIGN;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{'*'} && lexer.peek() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_MUL_ASSIGN;
		return;
	} else if (lexer.now() == UTF::U8Char{'/'} && lexer.peek() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_DIV_ASSIGN;
		return;
	} else if (lexer.now() == UTF::U8Char{'%'} && lexer.peek() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_MOD_ASSIGN;
		return;
	} else if (lexer.now() == UTF::U8Char{'^'}) {
		if (lexer.peek() == UTF::U8Char{'^'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LOGIC_XOR;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIT_XOR_ASSIGN;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{'&'}) {
		if (lexer.peek() == UTF::U8Char{'&'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LOGIC_AND;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIT_AND_ASSIGN;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{'|'}) {
		if (lexer.peek() == UTF::U8Char{'|'}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_LOGIC_OR;
			return;
		}
		if (lexer.peek() == UTF::U8Char{'='}) {
			tok.text.pushBack(lexer.next());
			tok.type = LTS_TT_BIT_OR_ASSIGN;
			return;
		}
	} else if (lexer.now() == UTF::U8Char{'!'} && lexer.now() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_COMPARE_NOT_EQUALS;
		return;
	} else if (lexer.now() == UTF::U8Char{'~'} && lexer.now() == UTF::U8Char{'='}) {
		tok.text.pushBack(lexer.next());
		tok.type = LTS_TT_BIT_NOT_ASSIGN;
		return;
	}
	tok.type = TokenStream::Token::Type{lexer.now().value()};
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
	while (isSpaceChar(lexer->next()) && !lexer->empty())
		lexer->next();
	if (lexer->empty()) {
		isFinished = true;
		return false;
	}
	UTF::U8String lexeme;
	curToken = {.at = position()};
	if (isNumberChar(lexer->now()) || (isOtherNumberChar(lexer->now()) && !isWordChar(lexer->peek()))) {
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
			err = Error{curToken.at, lexeme.toString()};
			isFinished = true;
		}
	}
	else if (lexer->now() == UTF::U8Char{'/'} && lexer->peek() == UTF::U8Char{'*'})
		parseBlockComment(*lexer);
	else if (lexer->now() == UTF::U8Char{'/'} && lexer->peek() == UTF::U8Char{'/'})
		parseLineComment(*lexer);
	else if (isWordChar(lexer->now())) {
		lexeme = parseID(*lexer);
		curToken.text = lexeme.toString();
		curToken.value = curToken.text.toString();
		curToken.type = LTS_TT_IDENTIFIER;
	}
	else if (closingQuote(lexer->now()) != UTF::U8Char{}) {
		curToken.type = stringType(lexer->now());
		lexer->next();
		lexeme = parseString(*lexer, closingQuote(lexer->now()));
		curToken.text = lexeme.toString();
		curToken.value = curToken.text.toString();
	}
	else parseOperator(*lexer, curToken);
	if (lexeme.size())
		curToken.text = lexeme.toString();
	if (lexer->empty())
		isFinished = true;
	DEBUGLN("Token: ", Token::asName(curToken.type));
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
		lexer->curCol,
		lexer->curLine
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
