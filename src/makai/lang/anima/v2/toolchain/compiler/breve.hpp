#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct Breve: ACompiler {
		struct Context: BaseContext {
		};

		void invoke() override;
	};
}

#endif
