#ifndef MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H
#define MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H

#include "../../../../compat/ctl.hpp"
#include "../core/instruction.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Context {
		struct Pointers {
			usize	offset		= 0;
			usize	function	= 0;
			usize	instruction	= 0;
		};
		using VariableBank = Map<ID::LUID, Data::Value>;
		Pointers				pointers;
		Data::Value::ArrayType	valueStack;
		List<Pointers>			pointerStack;
		VariableBank			globals;
		VariableBank			locals;
		VariableBank			arguments;
		Data::Value				temporary;
	};
}

#endif
