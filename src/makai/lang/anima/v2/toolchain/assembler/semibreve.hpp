#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_BREVE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_BREVE_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Semibreve: AAssembler {
		struct Context: BaseContext {};

		Semibreve(Context& context): AAssembler(context), context(context) {}
		void invoke() override;

		Context& context;
	};
}

#endif
