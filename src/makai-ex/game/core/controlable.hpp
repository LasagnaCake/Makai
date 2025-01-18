#ifndef MAKAILIB_EX_GAME_CORE_CONTROLABLE_H
#define MAKAILIB_EX_GAME_CORE_CONTROLABLE_H

#include <makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {
	/// @brief Controlable class base.
	struct Controllable {
		/// @brief Returns whether the button is down, or whether the button was just pressed.
		/// @param button Button to check for.
		/// @param justPressed Whether to check if button was just pressed.
		/// @return Whether button is down (`justPressed = false`) or button is just pressed (`justPressed = true`).
		bool action(String const& button, bool const justPressed = false) {
			if (!bindmap.contains(button)) return false;
			if (justPressed)
				return input.isButtonJustPressed(bindmap[button]);
			return input.isButtonDown(bindmap[button]);
		}

		/// @brief Returns the state of a button.
		/// @param action Button to check for.
		/// @return Button state. 
		usize actionState(String const& button) {
			if (!bindmap.contains(button)) return 0;
			return input.getButtonState(bindmap[button]);
		}

		/// @brief Input manager.
		Input::Manager		input;
		/// @brief Input bind map. used to get which button to check.
		Dictionary<String>	bindmap;
	};
}

#endif