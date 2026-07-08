#ifndef MAKAILIB_ANIMA_V2_CORE_METHOD_H
#define MAKAILIB_ANIMA_V2_CORE_METHOD_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct [[gnu::packed, gnu::aligned(1)]] MethodFlags {
		uint64 isExternal:	1 = false;
		uint64 isShared:	1 = false;
		uint64 isOptional:	1 = false;
	};

	struct Method: Entry, Flagged<MethodFlags> {
		Instance<Definition>		retType;
		List<Instance<Definition>>	argTypes;
	};
}

#endif
