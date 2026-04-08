#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPOSER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPOSER_H

#include "intermediate.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct Composer;

	struct Composer {
		Intermediate::Instance inter;

		UTF8String toMinima() const;
	};
}

#endif
