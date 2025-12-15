#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_EXPR_ARITHMETIC_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_EXPR_ARITHMETIC_H

#include "expression.hpp"


namespace Makai::Anima::V2::Toolchain::Compiler::Expr {
	struct Arithmetic: AExpression {
		List<ParseTree::Node> parse(usize const start, ParseTree::Node& parent) override;
	};
}

#endif