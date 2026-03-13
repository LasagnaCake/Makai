#ifndef MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H
#define MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H

#include "../../../../compat/ctl.hpp"
#include "../core/core.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Context {
		using Storage = Core::Object::Storage;

		struct IInvokable {
			virtual ~IInvokable() {}

			virtual Storage invoke(List<Storage> const& args) = 0;
		};

		struct Pointers {
			usize	offset		= 0;
			usize	function	= 0;
			usize	instruction	= -1;
		};

		struct Scope {
			List<Storage>		localStack;
			Core::ContextMode	mode		= Core::ContextMode::AV2_CM_STRICT;
			Core::ContextMode	prevMode	= Core::ContextMode::AV2_CM_STRICT;
		};

		using VariableBank = Map<uint64, Data::Value>;

		Pointers			pointers;
		List<Storage>		globalValueStack;
		List<Pointers>		pointerStack;
		List<Scope>			scopeStack;
		Map<usize, Storage>	globals;
		Core::Context		art;
	};
}

#endif
