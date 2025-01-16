#ifndef MAKAILIB_EX_GAME_CORE_CONTROLABLE_H
#define MAKAILIB_EX_GAME_CORE_CONTROLABLE_H

#include <makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {
	struct Controllable {
		bool action(String const& button, bool const justPressed = false) {
			if (justPressed)
				return input.isButtonJustPressed(bindmap[button]);
			return input.isButtonDown(bindmap[button]);
		}

		usize actionState(String const& button) {
			return input.getButtonState(bindmap[button]);
		}

		/// @brief Input manager.
		Input::Manager		input;
		/// @brief Input bind map.
		Dictionary<String>	bindmap;
	};
}

#endif