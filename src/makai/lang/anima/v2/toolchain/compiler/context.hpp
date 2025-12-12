#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CONTEXT_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CONTEXT_H

#include "parsetree.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct Context {
		using TokenList = Lexer::CStyle::TokenStream::TokenList;

		struct Scope {
			constexpr Scope(Context& ctx): ctx(ctx) {}

			constexpr ~Scope() {
				if (count) ctx.names.eraseRange(-Cast::as<ssize>(count), -1);
			}

			constexpr void addName(String const& name) {
				ctx.names.pushBack(name);
				++count;
			}

		private:
			Context&	ctx;
			usize		count = 0;
		};

		StringList	names;
		TokenList	tokens;
		ParseTree	tree;
	};
}

#endif