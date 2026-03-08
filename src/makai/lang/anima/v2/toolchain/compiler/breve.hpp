#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_H

#include "compiler.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct Breve: ACompiler {

		void invoke() override;
	};
}

#endif
