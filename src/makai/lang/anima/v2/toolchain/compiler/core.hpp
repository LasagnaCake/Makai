#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H

#include "../../runtime/runtime.hpp"
#include "../assembler/assembler.hpp"
#include "parser.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct ACompiler {
		using BaseContext = Assembler::BaseContext;
		virtual BaseContext::Input compile() = 0;
		virtual BaseContext::Input transpile() = 0;
	};
}

#endif
