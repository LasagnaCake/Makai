#ifndef MAKAILIB_ANIMA_V2_CORE_METHOD_H
#define MAKAILIB_ANIMA_V2_CORE_METHOD_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct Method {
		Instance<Definition>		retType;
		List<Instance<Definition>>	argTypes;
		String						name;
		bool						out = false;
	};
}

#endif
