#ifndef MAKAILIB_LEXER_CSTYLE_TOKENSTREAM_H
#define MAKAILIB_LEXER_CSTYLE_TOKENSTREAM_H

#include "../../file/file.hpp"

/// @brief C-Style language lexing facilities.
namespace Makai::Lexer::CStyle {
	/// @brief Token stream.
	struct TokenStream {
		DEFINE_ERROR_TYPE_EX(InvalidToken, InvalidValue);

		/// @brief Default string literal buffer size.
		constexpr static usize const DEFAULT_BUFFER_SIZE = 0x10000;

		/// @brief Token position in the stream.
		struct Position {
			/// @brief Index into the text.
			usize at;
			/// @brief Token line number.
			usize line;
			/// @brief Token column.
			usize column;
		};

		/// @brief Token stream error.
		struct Error {
			Position	where;
			String		token;
		};

		/// @brief Token stream token.
		struct Token {
			/// @brief Token type. May be a single unicode character, or one of the pre-defined values.
			enum class Type: usize {
				/// @brief Invalid token.
				LTS_TT_INVALID = Limit::MAX<usize>,
				/// @brief Integer literal.
				LTS_TT_INTEGER = Limit::MAX<byte> + 1,
				/// @brief Floating-point literal.
				LTS_TT_REAL,
				/// @brief Alphanumeric identifier.
				LTS_TT_IDENTIFIER,
				/// @brief Single-quote string literal.
				LTS_TT_SINGLE_QUOTE_STRING,
				/// @brief Double-quote string literal.
				LTS_TT_DOUBLE_QUOTE_STRING,
				/// @brief Character literal.
				LTS_TT_CHARACTER,
				/// @brief Equality comparison (`==`).
				LTS_TT_COMPARE_EQUALS,
				/// @brief Inequality comparison (`!=`).
				LTS_TT_COMPARE_NOT_EQUALS,
				/// @brief Lesser-or-equals comparison (`<=`).
				LTS_TT_COMPARE_LESS_EQUALS,
				/// @brief Greater-or-equals comparison (`>=`).
				LTS_TT_COMPARE_GREATER_EQUALS,
				/// @brief Logical AND (`&&`).
				LTS_TT_LOGIC_AND,
				/// @brief Logical OR (`||`).
				LTS_TT_LOGIC_OR,
				/// @brief Bit shift left (`>>`).
				LTS_TT_BIT_SHIFT_LEFT,
				/// @brief Bit shift right (`<<`).
				LTS_TT_BIT_SHIFT_RIGHT,
				/// @brief Increment (`++`).
				LTS_TT_INCREMENT,
				/// @brief Decrement (`--`).
				LTS_TT_DECREMENT,
				/// @brief Addition assignment (`+=`).
				LTS_TT_ADD_ASSIGN,
				/// @brief Subtraction assignment (`-=`).
				LTS_TT_SUB_ASSIGN,
				/// @brief Multiplication assignment (`*=`).
				LTS_TT_MUL_ASSIGN,
				/// @brief Division assignment (`/=`).
				LTS_TT_DIV_ASSIGN,
				/// @brief Modulo division assignment (`%=`).
				LTS_TT_MOD_ASSIGN,
				/// @brief Bitwise AND assignment (`&=`).
				LTS_TT_BIT_AND_ASSIGN,
				/// @brief Bitwise OR assignment (`|=`).
				LTS_TT_BIT_OR_ASSIGN,
				/// @brief Bitwise XOR assignment (`^=`).
				LTS_TT_BIT_XOR_ASSIGN,
				/// @brief "Little arrow" (`->`).
				LTS_TT_LITTLE_ARROW,
				/// @brief "Big arrow" (`=>`).
				LTS_TT_BIG_ARROW,
				/// @brief Bitwise shift left assignment (`<<=`).
				LTS_TT_BIT_SHIFT_LEFT_ASSIGN,
				/// @brief Bitwise shift right assignment (`>>=`).
				LTS_TT_BIT_SHIFT_RIGHT_ASSIGN,
				/// @brief Maximum token types.
				LTS_TT_MAX_TOKEN_TYPES,
			};

			/// @brief Token type.
			Type		type		= Type::LTS_TT_INVALID;
			/// @brief Token value.
			Data::Value	value;
		};

		/// @brief Token list.
		using TokenList = List<Token>;

		/// @brief Lexer implementation.
		struct Lexer;
		
		/// @brief Empty constructor.
		TokenStream();

		/// @brief Destructor.
		~TokenStream();

		/// @brief Opens the token stream.
		/// @param source Source content to process.
		/// @param bufferSize Size of buffer to process string literals. By default, it is `DEFAULT_BUFFER_SIZE`.
		/// @note Source is copied, so there's no need to keep it around.
		TokenStream(String const& source, usize const bufferSize = DEFAULT_BUFFER_SIZE);

		/// @brief Opens the token stream.
		/// @param source Source content to process.
		/// @param bufferSize Size of buffer to process string literals. By default, it is `DEFAULT_BUFFER_SIZE`.
		/// @return Reference to self.
		/// @note Source is copied, so there's no need to keep it around.
		TokenStream& open(String const& source, usize const bufferSize = DEFAULT_BUFFER_SIZE);

		/// @brief Closes the token stream.
		/// @return Reference to self.
		TokenStream& close();

		/// @brief Fetches the next token.
		/// @return Whether there is data to parse.
		bool next();

		/// @brief Returns the current token.
		/// @return Current token.
		constexpr Token current() const {return curToken;}

		/// @brief Returns the current token's text.
		/// @return Current token's text.
		String tokenText() const;

		/// @brief Returns the token stream's current position, INCLUDING line & column number.
		/// @return Current position.
		Position position() const;

		/// @brief Returns the token stream's current position, EXCLUDING line & column number.
		/// @return Current position.
		usize location() const;

		/// @brief Returns whether the token stream has finished processing.
		/// @return Whether stream is finished.
		bool finished() const;

		/// @brief Returns whether the token stream has not encountered an error.
		/// @return Whether stream has not encountered an error.
		constexpr bool ok() const {return !err.exists();}
		
		/// @brief Returns whether the token stream has not encountered an error.
		/// @return Whether stream has not encountered an error.
		operator bool() const {return finished();}

		/// @brief Returns the current error.
		/// @return Current error.
		constexpr Nullable<Error> error() const {return err;}

		/// @brief Asserts that the token stream has not encountered an error.
		/// @throw InvalidToken If an error was encountered.
		void assertOK() const;

	private:
		/// @brief Current fetched token.
		Token curToken;

		/// @brief Current error.
		Nullable<Error> err;

		/// @brief Whether token stream has finished processing.
		bool isFinished = false;

		/// @brief Lexer implementation.
		Unique<Lexer> lexer;
	};

	/// @brief Converts some source content to a list of tokens.
	/// @param source Source content to process.
	/// @param bufferSize Size of buffer to process string literals. By default, it is `TokenStream::DEFAULT_BUFFER_SIZE`.
	/// @return Resulting tokens, or an error (on failure).
	Result<TokenStream::TokenList, TokenStream::Error>
	tokenize(String const& source, usize const bufferSize = TokenStream::DEFAULT_BUFFER_SIZE);
}

#endif // MAKAILIB_LEXER_CORE_H