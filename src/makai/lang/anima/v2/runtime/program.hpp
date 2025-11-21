#ifndef MAKAILIB_ANIMA_V2_RUNTIME_PROGRAM_H
#define MAKAILIB_ANIMA_V2_RUNTIME_PROGRAM_H

#include "../core/instruction.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Program {
		Data::Value::ArrayType	types;
		Data::Value::ArrayType	constants;
		List<Core::Instruction>	code;
		List<usize>				jumpTable;
	};
}

#endif
