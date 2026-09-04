#ifndef MAKAILIB_MP_OCL_CONTEXT_H
#define MAKAILIB_MP_OCL_CONTEXT_H

#include "component.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	struct Context: Component<Context> {
		struct Impl;
	};
}

#endif
