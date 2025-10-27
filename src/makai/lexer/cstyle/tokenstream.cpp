#include "tokenstream.hpp"
#include <stb_c_lexer.h>

using namespace Makai::Lexer::CStyle;

struct TokenStream::Lexer {
	stb_lexer lexer;
	String const source;
	String buffer;
};

bool TokenStream::next() {
	if (!lexer || isFinished) return false;
	if (stb_c_lexer_get_token(&lexer->lexer)) {
		auto& lex = lexer->lexer;
		switch (lex.token) {
			case (CLEX_eof): isFinished = true; break;
			case (CLEX_parse_error): {
				stb_lex_location loc;
				stb_c_lexer_get_location(&lex, lex.where_firstchar, &loc);
				err = TokenStream::Error{
					position(),
					String(lex.where_firstchar, lex.where_lastchar)
				};
				curToken.type = Token::Type::LTS_TT_INVALID;
				isFinished = true;
				goto Skip;
			} break;
			case (CLEX_charlit): {
				curToken.value = static_cast<uint32>(lex.int_number);
			} break;
			case (CLEX_intlit): {
				curToken.value = lex.int_number < 0 ? static_cast<ssize>(lex.int_number) : static_cast<usize>(lex.int_number);
			} break;
			case (CLEX_floatlit): {
				curToken.value = lex.real_number;
			} break;
			case (CLEX_sqstring):
			case (CLEX_dqstring):
			case (CLEX_id): {
				if (lex.string_len > 0)
					curToken.value = String(lex.string, lex.string + lex.string_len);
				else curToken.value.clear();
			} break;
			default: break;
		}
		curToken.type = static_cast<Token::Type>((lex.token < 256) ? lex.token : lex.token - 1);
	} else isFinished = true;
Skip:
	return !isFinished;
}

TokenStream::Position TokenStream::position() const {
	if (!lexer) return {
		Limit::MAX<usize>,
		Limit::MAX<usize>,
		Limit::MAX<usize>
	};
	stb_lex_location loc;
	auto& lex = lexer->lexer;
	stb_c_lexer_get_location(&lex, lex.where_firstchar, &loc);
	return {
		location(),
		static_cast<usize>(loc.line_number),
		static_cast<usize>(loc.line_offset)
	};
}

usize TokenStream::location() const {
	if (!lexer) return -1;
	auto& lex = lexer->lexer;
	return static_cast<usize>(lex.where_firstchar - lex.input_stream);
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

TokenStream::TokenStream(String const& source, usize const bufferSize)	{open(source, bufferSize);	}
TokenStream::TokenStream()												{							}
TokenStream::~TokenStream()												{close();					}

TokenStream& TokenStream::open(String const& source, usize const bufferSize) {
	if (lexer) return *this;
	lexer.bind(new Lexer{{}, source});
	lexer->buffer.resize(bufferSize, '\0');
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
	if (!lexer) return *this;
	lexer.unbind();
	isFinished = true;
	return *this;
}

Makai::Result<TokenStream::TokenList, TokenStream::Error>
Makai::Lexer::CStyle::tokenize(Makai::String const& source, usize const bufferSize) {
	TokenStream::TokenList result;
	TokenStream stream{source, bufferSize};
	while (stream.next())
		result.pushBack(stream.current());
	if (!stream.ok()) return *stream.error();
	return result;
}