#ifndef MAKAILIB_ANIMA_V2_RUNTIME_MODEL_METHOD_H
#define MAKAILIB_ANIMA_V2_RUNTIME_MODEL_METHOD_H

#include "type.hpp"
#include "../../core/core.hpp"

namespace Makai::Anima::V2::Runtime {
	struct MethodModel: Entry, Flagged<Core::MethodFlags> {
		AtomicCell<TypeModel>		retType;
		List<AtomicCell<TypeModel>>	argTypes;
	};
}

#endif
