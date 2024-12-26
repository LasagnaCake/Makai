#ifndef MAKAILIB_CORE_DISPLAY_H
#define MAKAILIB_CORE_DISPLAY_H

#include "../compat/ctl.hpp"

/// @brief Makai core API.
namespace Makai {
	/// @brief Screen position.
	struct ScreenPosition {
		/// @brief Location.
		int x, y;
	};

	/// @brief Returns the size of the given display.
	/// @param display Display to query size.
	/// @return Display size.
	Vector2 getDisplaySize(usize const display = 0);
}

#endif // MAKAILIB_CORE_DISPLAY_H
