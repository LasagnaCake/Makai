#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPOSER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPOSER_H

#include "intermediate.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct Composer;

	struct Composer {
		Map<Namespace::Instance, bool> visited;

		Intermediate::Instance inter;

		UTF8StringList types;
		UTF8StringList functions;

		usize staticVarCount = 0;

		Implementation impl;

		UTF8String toMinima() const;
	};
}

#endif
