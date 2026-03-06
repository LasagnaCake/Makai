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
			auto const pos = tokens.back().position;
			throw E(
				Makai::toString(
					"At:\nLINE: ", pos.line,
					"\nCOLUMN: ", pos.column,
					"\n--> [", tokens.back().token, "]"
				),
				what,
				Makai::CPP::SourceFile{"n/a", Cast::as<int>(pos.line), tokens.back().sourceFile.size() ? tokens.back().sourceFile : fileName}
			);
		}

		struct MessageOutput {
			template <class... Args>
			void write(Args const&... args)		{(..., display(toString(args)));				}
			template <class... Args>
			void writeLine(Args const&... args)	{(..., display(toString(args))); display("\n");	}

		protected:
			virtual void display(String const& str)	{DEBUG(str);	}
		};

		MessageOutput&	out;

		BaseContext(MessageOutput& out = defaultWriter): out(out) {}

		BaseContext& next() {
			if (tokens.empty())
				error("Unexpected end-of-file!");
			tokens.popBack();
			return *this;
		}

		BaseContext& append(Input const& content) {
			tokens.insert(content.reversed(), 0);
			return *this;
		}

		BaseContext& pad(Input const& content) {
			tokens.appendBack(content.reversed());
			return *this;
		}

		BaseContext& pad(usize const count) {
			if (!count) return *this;
			if (count == 1) tokens.pushBack({});
			else tokens.appendBack(Input().resize(count, {}));
			return *this;
		}

		bool has(Axiom::Type const& type) {
			return tokens.back().type == type;
		}

		void expect(Axiom::Type const& type, String const& what) {
			if (!has(type))
				error();
		}

		bool get(Axiom::Type const& type, String const& what) {
			return tokens.back().type == type;
		}

	private:
		Input tokens;

		inline static MessageOutput defaultWriter;
	};
}

#endif
