#include "tokenstream.hpp"
#include <stb_c_lexer.h>

using namespace Makai::Lexer::CStyle;

struct TokenStream::Lexer {
	stb_lexer lexer;
	String const source;
	String buffer;
};

bool TokenStream::next() {
	if (isFinished) return false;
	if (stb_c_lexer_get_token(&lexer->lexer)) {
		auto& lex = lexer->lexer;
		switch (lex.token) {
			case (CLEX_eof): isFinished = true; break;
			case (CLEX_parse_error): {
				stb_lex_location loc;
				stb_c_lexer_get_location(&lex, lex.where_firstchar, &loc);
				err = TokenStream::Error{
					loc.line_number,
					loc.line_offset,
					String(lex.where_firstchar, lex.where_lastchar)
				};
				curToken.type = Token::Type::LTS_TT_INVALID;
				isFinished = true;
			} break;
			case (CLEX_charlit): {
				curToken.value = UTF8Char(static_cast<uint32>(lex.int_number));
			} goto TheRestOfTheOwl;
			case (CLEX_intlit): {
				curToken.value = static_cast<ssize>(lex.int_number);
			} goto TheRestOfTheOwl;
			case (CLEX_floatlit): {
				curToken.value = lex.real_number;
			} goto TheRestOfTheOwl;
			case (CLEX_sqstring):
			case (CLEX_dqstring): {
				curToken.value = String(lex.string, lex.string_len);
			} goto TheRestOfTheOwl;
			default: {
			TheRestOfTheOwl:
				curToken.type = static_cast<Token::Type>((lex.token < 256) ? lex.token : lex.token - 1);
			} break;
		}
	} else isFinished = true;
	return !isFinished;
}

void TokenStream::assertOK() const {
	if (err)
		throw TokenStream::InvalidToken(
			toString(
				"Invalid token \"", err.value().token, "\"!"
				"\nAt line [", err.value().line,"], column [", err.value().column,"]"
			),
			CTL_CPP_PRETTY_SOURCE
		);
}

TokenStream::TokenStream(String const& source)	{open(source);	}
TokenStream::TokenStream()						{				}
TokenStream::~TokenStream()						{close();		}

TokenStream& TokenStream::open(String const& source) {
	if (lexer) return *this;
	lexer.bind(new Lexer{{}, source});
	lexer->buffer.resize(4096, '\0');
	stb_c_lexer_init(
		&lexer->lexer,
		lexer->source.begin(), lexer->source.end(),
		lexer->buffer.data(), lexer->buffer.size()
	);
	err = nullptr;
	isFinished = false;
	return *this;
}

TokenStream& TokenStream::close() {
	if (lexer) return *this;
	lexer.unbind();
	isFinished = true;
	return *this;
}