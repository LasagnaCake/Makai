#ifndef MAKAILIB_ANIMA_V2_RUNTIME_MODEL_METHOD_H
#define MAKAILIB_ANIMA_V2_RUNTIME_MODEL_METHOD_H

#include "type.hpp"
#include "../../core/flags.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Method: Entry, Flagged<Core::MethodFlags> {
		AtomicCell<Definition>			retType;
		List<AtomicCell<Definition>>	argTypes;
	};
}

#endif
