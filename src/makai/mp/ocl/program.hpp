#ifndef MAKAILIB_MP_OCL_PROGRAM_H
#define MAKAILIB_MP_OCL_PROGRAM_H

#include "component.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	struct Program: Component<Program> {
		struct Impl;
	};
}

#endif
