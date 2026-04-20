#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPILER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPILER_H

#include "../core.hpp"
#include "parser.hpp"
#include "intermediate.hpp"
#include "node.hpp"
#include "transformer.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	enum class CompilationLevel {
		AV2_TCB_CCL_PARSE_TREE,
		AV2_TCB_CCL_INTERMEDIATE,
		AV2_TCB_CCL_MINIMA,
		AV2_TCB_CCL_FULL,
	};

	Data::Value compile(UTF8String const& fname, UTF8String const& file, CompilationLevel const level);
}

#endif
