#ifndef MAKAILIB_ANIMA_V2_CORE_FLAGS_H
#define MAKAILIB_ANIMA_V2_CORE_FLAGS_H

#include "forward.hpp"

namespace Makai::Anima::V2::Core {
	struct [[CTL_FLAG_STRUCT(uint64)]] TypeFlags {
		uint64 isBasic:		1 = false;
		uint64 isNullable:	1 = false;
		uint64 isEmpty:		1 = false;
		uint64 hasNoResult:	1 = false;
		uint64 isArray:		1 = false;
		uint64 isValueType:	1 = false;
		uint64 isStructure:	1 = false;
		uint64 isDynamic:	1 = false;
		uint64 isCopyable:	1 = false;
		uint64 isProxy:		1 = false;
		uint64 isPointer:	1 = false;
		uint64 isFinal:		1 = false;
		uint64 isEnum:		1 = false;
		uint64 isFunction:	1 = false;
		uint64 isUnion:		1 = false;
		uint64 isGeneric:	1 = false;
		CTL_FLAG_STRUCT_END(uint64);
	};

	static_assert(sizeof(TypeFlags) == sizeof(uint64), "Uh oh :/");

	struct [[CTL_FLAG_STRUCT(uint64)]] MethodFlags {
		uint64 isExternal:	1 = false;
		uint64 isShared:	1 = false;
		uint64 isFFI:		1 = false;
		uint64 isOptional:	1 = false;
		uint64 isGeneric:	1 = false;
		CTL_FLAG_STRUCT_END(uint64);
	};

	static_assert(sizeof(MethodFlags) == sizeof(uint64), "Uh oh :/");
}

#endif
