#ifndef MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H
#define MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H

#include "../../../../compat/ctl.hpp"
#include "../instruction.hpp"

#define ANIMA_V2_SHARED_FN_NAME_PREFIX "anima/v2/env/share/fn/"

namespace Makai::Anima::V2::Runtime {
	constexpr auto const SHARED_FUNCTION_PREFIX = ANIMA_V2_SHARED_FN_NAME_PREFIX;
	struct Context {
		struct Pointers {
			usize	offset		= 0;
			usize	function	= 0;
			usize	instruction	= -1;
		};
		using VariableBank = Map<uint64, Data::Value>;
		ContextMode						mode		= ContextMode::AV2_CM_STRICT;
		ContextMode						prevMode	= ContextMode::AV2_CM_STRICT;
		Pointers						pointers;
		Data::Value::ArrayType			valueStack;
		List<Pointers>					pointerStack;
		Data::Value::ArrayType			globals;
		As<Data::Value[REGISTER_COUNT]>	registers;
		Data::Value						temporary;
		Dictionary<CPP::Library>		shared;
	};
}

#define ANIMA_V2_SHARED_FN(NAME, EXPORT)\
	Makai::Data::Value NAME(Makai::Data::Value::ArrayType) asm(ANIMA_V2_SHARED_FN_NAME_PREFIX EXPORT);\
	Makai::Data::Value NAME(Makai::Data::Value::ArrayType args)

#endif
