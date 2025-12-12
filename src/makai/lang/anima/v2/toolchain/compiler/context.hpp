#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CONTEXT_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CONTEXT_H

#include "parsetree.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct Context {
		using TokenStream	= Lexer::CStyle::TokenStream;
		using TokenList		= Lexer::CStyle::TokenStream::TokenList;

		enum class NameType {
			AV2_TC_CNT_RESERVED,
			AV2_TC_CNT_VARIABLE,
			AV2_TC_CNT_FUNCTION,
			AV2_TC_CNT_TYPE
		};

		struct Scope {
			constexpr Scope(Context& ctx): ctx(ctx) {}

			constexpr ~Scope() {
				for (auto const& name: scopeNames)
					ctx.names.erase(name);
			}

			constexpr void addName(String const& name, NameType const type) {
				ctx.names[name] = type;
				scopeNames.pushBack(name);
			}

		private:
			Context&	ctx;
			StringList	scopeNames;
		};

		Dictionary<NameType>	names;
		TokenList				tokens;
		ParseTree				tree;
	};
}

#endif