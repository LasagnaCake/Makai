#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CONTEXT_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CONTEXT_H

#include "../../../../../lexer/lexer.hpp"
#include "../../runtime/program.hpp"
#include "../../core/instruction.hpp"
#include "../../core/type.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct BaseContext {
		struct Axiom;

		using Tokenizer	= Lexer::CStyle::TokenStream;
		using Program	= Runtime::Program;
		using Input		= List<Axiom>;

		virtual ~BaseContext() {}

		struct Axiom: Tokenizer::Token {
			bool strict = false;
			String token;
			Tokenizer::Position position = {0, 0, 0};
			String sourceFile;

			constexpr Ordered::OrderType operator<=>(Axiom const& other) const {
				if (!strict) return type <=> other.type;
				Ordered::OrderType order = type <=> other.type;
				if (order == Ordered::Order::EQUAL) return value <=> other.value;
				else return order;
			}

			constexpr bool operator==(Axiom const& other) const {
				if (!strict) return type == other.type;
				if (type == other.type)
					return value == other.value;
				else return false;
			}

			constexpr Ordered::OrderType operator<=>(Tokenizer::Token const& other) const {
				return operator<=>(Axiom{other});
			}

			constexpr bool operator==(Tokenizer::Token const& other) const {
				return operator==(Axiom{other});
			}
		};

		template <Type::Derived<Error::Generic> E = Error::InvalidValue>
		[[noreturn]]
		void error(String const& what) const {
			auto const pos = token().position;
			throw E(
				Makai::toString(
					"At:\nLINE: ", pos.line,
					"\nCOLUMN: ", pos.column,
					"\n--> [", token().token, "]"
				),
				what,
				Makai::CPP::SourceFile{"n/a", Cast::as<int>(pos.line), token().sourceFile}
			);
		}

		struct Messager {
			template <class... Args>
			void write(Args const&... args)		{(..., display(toString(args)));				}
			template <class... Args>
			void writeLine(Args const&... args)	{(..., display(toString(args))); display("\n");	}

		protected:
			virtual void display(String const& str)	{DEBUG(str);	}
		};

		Messager& out;

		BaseContext(Messager& out = defaultWriter): out(out) {}

		BaseContext& next() {
			if (tokens.empty())
				error("Unexpected end-of-file!");
			tokens.popBack();
			if (tokens.empty())
				error("Unexpected end-of-file!");
			return *this;
		}

		Axiom& token() 				{return tokens.back();}
		Axiom const& token() const	{return tokens.back();}

		BaseContext& append(Input const& content) {
			tokens.insert(content.reversed(), 0);
			return *this;
		}

		BaseContext& put(Input const& content) {
			tokens.appendBack(content.reversed());
			return *this;
		}

		BaseContext& pad(usize const count) {
			if (!count) return *this;
			if (count == 1) tokens.pushBack({});
			else put(Input().resize(count, {}));
			return *this;
		}

		bool has(Axiom::Type const& type) {
			return tokens.back().type == type;
		}

		BaseContext& expect(Axiom::Type const& type, String const& what) {
			if (!has(type))
				error("Expected " + what + " here!");
			return *this;
		}

		Data::Value& get(Axiom::Type const& type, String const& what) {
			if (!has(type))
				error("Expected " + what + " here!");
			return token().value;
		}

		BaseContext& expectNext(Axiom::Type const& type, String const& what) {
			return next().expect(type, what);
		}

		Data::Value& getNext(Axiom::Type const& type, String const& what) {
			return next().get(type, what);
		}

		BaseContext& expect(Axiom::Type const& type) {
			return expect(type, "'" + Axiom::asName(type) + "'");
		}

		Data::Value& get(Axiom::Type const& type) {
			return get(type, "'" + Axiom::asName(type) + "'");
		}

		Data::Value& getNext(Axiom::Type const& type) {
			return getNext(type, "'" + Axiom::asName(type) + "'");
		}

		BaseContext& expectNext(Axiom::Type const& type) {
			return expectNext(type, "'" + Axiom::asName(type) + "'");
		}

		Data::Value value() const {
			return token().value;
		}

		struct FileInfo {
			String name;
			String source;
		} file;

	private:
		Input tokens;

		inline static Messager defaultWriter;
	};
}

#endif
