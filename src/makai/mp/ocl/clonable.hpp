#ifndef MAKAILIB_MP_OCL_CLONABLE_H
#define MAKAILIB_MP_OCL_CLONABLE_H

#include "../../compat/ctl.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	template <Type::Class T>
	struct Clonable: T {
		T clone();
	};
}

#endif
