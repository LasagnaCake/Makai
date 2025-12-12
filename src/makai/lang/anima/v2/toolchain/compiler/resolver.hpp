#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_RESOLVER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_RESOLVER_H

#include "context.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct AResolver {
		using TokenStream	= Context::TokenStream;
		using TokenList		= Context::TokenList;

		AResolver(Context& ctx): context(ctx) {}

		virtual ~AResolver() {}

		virtual usize resolve(usize const start, ParseTree::Node& root) = 0;
	
	protected:
		Context& context;
	};
}

#endif