#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_VARIABLERESOLVER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_VARIABLERESOLVER_H

#include "../../../../../lexer/lexer.hpp"
#include "resolver.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct VariableResolver: AResolver {
		using AResolver::AResolver;

		virtual usize resolve(usize const start, ParseTree::Node& root) override;
	};
}

#endif