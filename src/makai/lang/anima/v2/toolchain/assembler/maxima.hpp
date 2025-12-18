#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MAXIMA_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MAXIMA_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Maxima: AAssembler {
		void assemble() override;
	};
}

#endif