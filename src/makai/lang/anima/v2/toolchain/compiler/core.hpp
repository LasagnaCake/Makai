#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H

#include "../assembler/assembler.hpp"
#include "intermediate.hpp"
#include "parser.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct ACompiler: IInvokable<void()>, IValue<Intermediate> {
		Parser& parser;
		BaseContext& context;
		ACompiler(Parser& parser, BaseContext& context);
	};
}

#endif
