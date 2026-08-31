#ifndef MAKAILIB_VIDEO_CORE_H
#define MAKAILIB_VIDEO_CORE_H

#include "../compat/ctl.hpp"
#include "../image/core.hpp"
#include "get.hpp"

namespace Makai::Video::V2D {
	enum class Container {
		MV2F_INVALID = -1,
		MV2F_UNKNOWN,
		/// @brief Quite OK Video.
		MV2F_QOV,
		/// @brief Makai Video Storage & eXcgange format.
		MV2F_MVSX,
	};

	struct Attributes {
		Container container;
	};
}
