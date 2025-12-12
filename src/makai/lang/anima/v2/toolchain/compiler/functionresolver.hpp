#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_FUNCTIONRESOLVER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_FUNCTIONRESOLVER_H

#include "resolver.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct FunctionResolver: AResolver {
		using AResolver::AResolver;

		virtual usize resolve(usize const start, ParseTree::Node& root) override;
	};
}

#endif