#ifndef MAKAILIB_ANIMA_V2_CORE_METHOD_H
#define MAKAILIB_ANIMA_V2_CORE_METHOD_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct Method: Entry {
		struct Flags {
			constexpr static uint64 const AV2_MF_EXTERNAL	= 1 << 0;
			constexpr static uint64 const AV2_MF_SHARED		= 1 << 1;
			constexpr static uint64 const AV2_MF_OPTIONAL	= 1 << 2;
		};

		Instance<Definition>		retType;
		List<Instance<Definition>>	argTypes;
	};
}

#endif
