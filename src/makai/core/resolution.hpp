#ifndef MAKAILIB_CORE_RESOLUTION_H
#define MAKAILIB_CORE_RESOLUTION_H

#include "../compat/ctl.hpp"

/// @brief Resolution sets.
namespace Makai::Resolution {
	#define MAKAILIB_RESOLUTION(W, H) KeyValuePair<String, Vector2>{#W" : "#H , Vector2(W, H)}
	/// @brief 4 x 3 resolutions.
	const List<KeyValuePair<String, Vector2>> set4x3 = {
		MAKAILIB_RESOLUTION(480, 320),
		MAKAILIB_RESOLUTION(640, 480),
		MAKAILIB_RESOLUTION(860, 640),
		MAKAILIB_RESOLUTION(960, 720),
		MAKAILIB_RESOLUTION(1280, 960),
		MAKAILIB_RESOLUTION(1600, 1200)
	};

	/// @brief 16 x 9 resolutions.
	const List<KeyValuePair<String, Vector2>> set16x9 = {
		MAKAILIB_RESOLUTION(640, 360),
		MAKAILIB_RESOLUTION(854, 480),
		MAKAILIB_RESOLUTION(1280, 720),
		MAKAILIB_RESOLUTION(1600, 900),
		MAKAILIB_RESOLUTION(1920, 1080),
		MAKAILIB_RESOLUTION(2560, 1440)
	};
	#undef MAKAILIB_RESOLUTION
}

#endif // MAKAILIB_CORE_RESOLUTION_H
