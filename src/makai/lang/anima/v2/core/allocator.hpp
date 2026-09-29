#ifndef MAKAILIB_ANIMA_V2_CORE_ALLOCATOR_H
#define MAKAILIB_ANIMA_V2_CORE_ALLOCATOR_H

#include "../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Core {
	template <class T>
	using Allocator = GSPAllocator<T>;
}

#endif
