#ifndef MAKAILIB_ANIMA_V2_CORE_ENTRY_H
#define MAKAILIB_ANIMA_V2_CORE_ENTRY_H

#include "forward.hpp"

namespace Makai::Anima::V2::Core {
	struct Entry {
		uint64 id;
		String name;
		uint64 hash;
	};
}

#endif
