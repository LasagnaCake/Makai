#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_EXPR_EXPRESSION_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_EXPR_EXPRESSION_H

#include "../context.hpp"
#include "../parsetree.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Expr {
	struct AExpression {
		using TokenStream	= Context::TokenStream;
		using TokenList		= Context::TokenList;

		AExpression(Context& ctx): context(ctx) {}

		virtual List<ParseTree::Node> parse(usize const start, ParseTree::Node& parent) = 0;

		virtual ~AExpression() {}
	
	protected:
		Context& context;
	};
}

#endif