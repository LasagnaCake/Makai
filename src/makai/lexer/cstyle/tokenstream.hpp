#ifndef MAKAILIB_LEXER_CSTYLE_TOKENSTREAM_H
#define MAKAILIB_LEXER_CSTYLE_TOKENSTREAM_H

#include "../../file/file.hpp"

/// @brief C-Style language lexing facilities.
namespace Makai::Lexer::CStyle {
	/// @brief Token stream.
	struct TokenStream {
		DEFINE_ERROR_TYPE_EX(InvalidToken, InvalidValue);

		/// @brief Token stream error.
		struct Error {
			usize	line;
			usize	column;
			String	token;
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
			};

			/// @brief Primitive value variant.
			struct Value {
				/// @brief Value type.
				enum class Type {
					LTS_TVT_EMPTY,
					LTS_TVT_INTEGER,
					LTS_TVT_REAL,
					LTS_TVT_STRING,
					LTS_TVT_CHARACTER,
				};

				/// @brief Default constructor.
				constexpr Value() {}

				/// @brief Copy constructor.
				constexpr Value(Value const& other)	{
					switch (valType = other.valType) {
						case (Type::LTS_TVT_INTEGER):	integer		= other.integer; break;
						case (Type::LTS_TVT_REAL):		real		= other.real; break;
						case (Type::LTS_TVT_STRING):	string		= other.string; break;
						case (Type::LTS_TVT_CHARACTER): character	= other.character; break;
						default: break;
					}	
				}

				/// @brief Copy assignment operator.
				constexpr Value& operator=(Value const& other)	{
					switch (valType = other.valType) {
						case (Type::LTS_TVT_INTEGER):	integer		= other.integer; break;
						case (Type::LTS_TVT_REAL):		real		= other.real; break;
						case (Type::LTS_TVT_STRING):	string		= other.string; break;
						case (Type::LTS_TVT_CHARACTER): character	= other.character; break;
						default: break;
					}
					return *this;
				}

				/// @brief Destructor.
				constexpr ~Value() {
					switch (valType) {
						case Type::LTS_TVT_STRING:		string.~String(); break;
						case Type::LTS_TVT_CHARACTER:	character.~UTF8Char(); break;
						default: break;
					}
				}

				/// @brief Assigns an integer value to the variant.
				constexpr Value& operator=(ssize const v) {
					integer = v;
					valType = Type::LTS_TVT_INTEGER;
					return *this;
				}
				
				/// @brief Assigns a floating-point value to the variant.
				constexpr Value& operator=(double const v) {
					real = v;
					valType = Type::LTS_TVT_REAL;
					return *this;
				}

				/// @brief Assigns a string to the variant.
				constexpr Value& operator=(String const& v) {
					string = v;
					valType = Type::LTS_TVT_STRING;
					return *this;
				}
				
				/// @brief Assigns a UTF-8 character to the variant.
				constexpr Value& operator=(UTF8Char const v) {
					character = v;
					valType = Type::LTS_TVT_CHARACTER;
					return *this;
				}

				/// @brief Returns the value stored in the variant, or `nullptr`.
				/// @tparam T Type to get. Must be a valid `Type` enum value.
				/// @return Value stored in the variant. Returns `nullptr` if value's type does not match the requested type `T`.
				template <Type T>
				constexpr Nullable<Meta::NthType<enumcast(T) - 1, ssize, double, String, UTF8Char>>
				get() requires (T != Type::LTS_TVT_EMPTY) {
					if (valType == T) {
						if constexpr (T == Type::LTS_TVT_INTEGER)			return integer;
						else if constexpr (T == Type::LTS_TVT_REAL)			return real;
						else if constexpr (T == Type::LTS_TVT_STRING)		return string;
						else if constexpr (T == Type::LTS_TVT_CHARACTER)	return character;
					} else return nullptr;
				}

				/// @brief Returns the type of the value stored in the variant.
				/// @return Type of value.
				constexpr Type type() const		{return valType;						}

				/// @brief Returns whether the variant is empty.
				/// @return Whether variant is empty.
				constexpr bool empty() const	{return valType == Type::LTS_TVT_EMPTY;	}

			private:
				/// @brief Current value type.
				Type valType = Type::LTS_TVT_EMPTY;

				union {
					/// @brief Integer value, if an integer literal.
					ssize		integer;
					/// @brief Floating-point value, if a floating-point literal.
					double		real;
					/// @brief String value, if a string literal.
					String		string;
					/// @brief Character value, if a character literal.
					UTF8Char	character;
				};
			};

			/// @brief A token position in a file.
			struct Position {
				/// @brief Token line number.
				usize line;
				/// @brief Token column.
				usize column;
			};

			/// @brief Token type.
			Type	type		= Type::LTS_TT_INVALID;
			/// @brief Token value.
			Value	value;
		};

		/// @brief Lexer implementation.
		struct Lexer;
		
		/// @brief Empty constructor.
		TokenStream();

		/// @brief Destructor.
		~TokenStream();

		/// @brief Opens the token stream.
		/// @param source Source content to process.
		/// @note Source is copied to an internal buffer, so there's no need to keep it around.
		TokenStream(String const& source);

		/// @brief Opens the token stream.
		/// @param source Source content to process.
		/// @return Reference to self.
		/// @note Source is copied to an internal buffer, so there's no need to keep it around.
		TokenStream& open(String const& source);

		/// @brief Closes the token stream.
		/// @return Reference to self.
		TokenStream& close();

		/// @brief Fetches the next token.
		/// @return Whether no more tokens could be fetched.
		bool next();

		/// @brief Returns the current token.
		/// @return Current token.
		constexpr Token current() const {return curToken;}

		/// @brief Returns whether the token stream has finished processing.
		/// @return Whether stream is finished.
		bool finished() const;

		/// @brief Returns whether the token stream has not encountered an error.
		/// @return Whether stream has not encountered an error.
		constexpr bool ok() const {return !err.exists();}
		
		/// @brief Returns whether the token stream has not encountered an error.
		/// @return Whether stream has not encountered an error.
		constexpr operator bool() const {return ok();}

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
}

#endif // MAKAILIB_LEXER_CORE_H