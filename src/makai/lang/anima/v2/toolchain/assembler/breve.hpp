#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_BREVE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_BREVE_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Breve: AAssembler {
		struct Context: BaseContext {};

		Breve(Context& context): AAssembler(context), context(context) {}
		void assemble() override;

		Context& context;
	};
}

#endif
