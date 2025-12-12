#ifndef MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H
#define MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H

#include "../../../../compat/ctl.hpp"
#include "../instruction.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Context {
		struct Pointers {
			usize	offset		= 0;
			usize	function	= 0;
			usize	instruction	= -1;
		};
		using VariableBank = Map<uint64, Data::Value>;
		Pointers						pointers;
		Data::Value::ArrayType			valueStack;
		List<Pointers>					pointerStack;
		Data::Value::ArrayType			globals;
		As<Data::Value[REGISTER_COUNT]>	registers;
		Data::Value						temporary;
	};
}

#endif
