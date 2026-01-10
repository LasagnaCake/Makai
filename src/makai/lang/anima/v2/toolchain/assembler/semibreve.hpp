#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_SEMIBREVE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_SEMIBREVE_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Semibreve: AAssembler {
		using AAssembler::AAssembler;
		void assemble() override;
	};
}

#endif