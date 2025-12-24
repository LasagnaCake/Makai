#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H

#include "../../runtime/runtime.hpp"
#include "../assembler/assembler.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	inline Runtime::Program buildBreve(Makai::String const& source, Makai::String const& filename = "unknown") {
		Assembler::Context context;
		context.fileName = filename;
		context.stream.open(source);
		Assembler::Breve assembler(context);
	}
}

#endif