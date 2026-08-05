#ifndef MAKAILIB_ANIMA_V2_CORE_METHOD_H
#define MAKAILIB_ANIMA_V2_CORE_METHOD_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct [[CTL_FLAG_STRUCT(uint64)]] MethodFlags {
		uint64 isExternal:	1 = false;
		uint64 isShared:	1 = false;
		uint64 isOptional:	1 = false;
		CTL_FLAG_STRUCT_END(uint64);
	};

	static_assert(sizeof(MethodFlags) == sizeof(uint64), "Uh oh :/");

	struct Method: Entry, Flagged<MethodFlags> {
		AtomicCell<Definition>			retType;
		List<AtomicCell<Definition>>	argTypes;
	};
}

#endif
