#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H

#include "../assembler/assembler.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct ACompiler: Assembler::AAssembler {
		using AAssembler::AAssembler;
	};
}

#endif
