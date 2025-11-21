#ifndef MAKAILIB_ANIMA_V2_RUNTIME_FUNCTION_H
#define MAKAILIB_ANIMA_V2_RUNTIME_FUNCTION_H

#include "../core/instruction.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Function {
		struct Argument {
			using Type = Core::Instruction::Invocation::Parameter;
			Type	type;
			uint64	id;
		};
		uint64 name;
	};
}

#endif
