#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_SEMINIMA_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_SEMINIMA_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Seminima: AAssembler {
		void assemble() override;
	};
}

#endif