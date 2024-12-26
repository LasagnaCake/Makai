#ifndef MAKAI_CORE_INPUT_MANAGER_H
#define MAKAI_CORE_INPUT_MANAGER_H

#include "buttons.hpp"
#include "../../compat/ctl.hpp"
#include "../extern.hpp"
#include "../display.hpp"

/// @brief User input facilities.
namespace Makai::Input {
	/// @brief Mouse state.
	struct MouseState {
		/// @brief Current mouse position.
		ScreenPosition pos;
		/// @brief Last mouse position.
		ScreenPosition last;
	};

	/// @brief Input mask.
	enum class InputMask: ubyte {
		IM_NONE				= 0b000,
		IM_KEY				= 0b001,
		IM_MOUSE			= 0b010,
		IM_JOY				= 0b100,
		IM_KEY_AND_MOUSE	= 0b011,
		IM_JOY_AND_MOUSE	= 0b110,
		IM_JOY_AND_JOY		= 0b101,
		IM_ALL				= 0b111,
	};

	/// @brief Input manager.
	/// @warning Controller input is currently unimplemented!
	class Manager {
		using KeyBuffer			= Map<KeyCode, usize>;
		using MouseButtonBuffer	= Map<MouseCode, usize>;

		struct MouseBuffer {
			MouseState global;
			MouseState local;
			MouseState relative;
			MouseButtonBuffer buffer, last;
		};

	public:
		/// @brief Empty Constructor.
		Manager() {}

		/// @brief
		///		Updates the input manager.
		///		Not required to call every cycle, as the program already handles it.
		static void update();

		/// @brief Returns the mouse position relative to the target window.
		/// @return Mouse position relative to window.
		Vector2 getWindowMousePosition();

		/// @brief Returns the mouse position relative to the desktop.
		/// @return Mouse position relative to desktop.
		Vector2 getDesktopMousePosition();

		/// @brief Returns which direction the mouse is moving towards.
		/// @return Mouse direction.
		Vector2 getMouseDirection();

		/// @brief Sets mouse capturing state on the target window.
		/// @param enabled `true` = enabled, `false` = disabled.
		/// @param hideCursor Whether the cursor should not be visible.
		static void setMouseCapturing(bool const enabled = true, bool const hideCursor = true);

		/// @brief Sets the cursor visibility while in the target window.
		/// @param visible `true` = visible, `false` = hidden.
		static void setCursorVisibility(bool const visible = true);

		/// @brief Returns the given button's state.
		/// @param button Button to check.
		/// @return State of the button.
		/// @note
		///		- 0		→ Unpressed;
		/// 
		///		- 1+	→ Pressed;
		/// 
		/// Recommended if time pressed is required.
		/// @warning Controller input is currently unimplemented!
		usize getButtonState(Button const& button);

		/// @brief Returns whether the given button is pressed.
		/// @param button Button to check.
		/// @return Whether it is presed.
		/// @warning Controller input is currently unimplemented!
		bool isButtonDown(Button const& button);

		/// @brief Returns whether the given button has just been pressed.
		/// @param button Button to check.
		/// @return Whether it has just been presed.
		/// @warning Controller input is currently unimplemented!
		bool isButtonJustPressed(Button const& button);

		/// @brief Returns whether the given button has just been released.
		/// @param button Button to check.
		/// @return Whether it has just been presed.
		/// @warning Controller input is currently unimplemented!
		bool isButtonJustReleased(Button const& button);

		/// @brief Returns if the button is held (state > threshold).
		/// @param button Button to check.
		/// @return Whether it is currently being held.
		/// @warning Controller input is currently unimplemented!
		bool isButtonHeld(Button const& button);

		/// @brief Returns if the button's state has changed.
		/// @param button Button to check.
		/// @return Whether its state has changed.
		/// @warning Controller input is currently unimplemented!
		bool hasButtonChanged(Button const& button);

		/// @brief Returns all buttons who are currently pressed.
		/// @return All buttons currently pressed.
		/// @warning Controller input is currently unimplemented!
		List<Button> getButtonsDown();

		/// @brief Returns all buttons whose states just pressed.
		/// @return All buttons whose state just changed.
		/// @warning Controller input is currently unimplemented!
		List<Button> getButtonsChanged();

		/// @brief Returns all buttons who have been recently just pressed.
		/// @return All buttons recently just pressed.
		/// @warning Controller input is currently unimplemented!
		List<Button> getButtonsJustPressed();

		/// @brief Returns all buttons who have been recently just released.
		/// @return All buttons recently just released.
		/// @warning Controller input is currently unimplemented!
		List<Button> getButtonsJustReleased();

		/// @brief Returns all buttons who are currently held.
		/// @return All buttons currently held.
		/// @warning Controller input is currently unimplemented!
		List<Button> getButtonsHeld();

		/// @brief Returns the given button's state.
		/// @param button Button to check.
		/// @return State of the button.
		/// @note
		///		- 0		→ Unpressed;
		/// 
		///		- 1+	→ Pressed;
		/// 
		/// Recommended if time pressed is required.
		/// @warning Controller input is currently unimplemented!
		usize getButtonState(String const& button);

		/// @brief Returns whether the given button is pressed.
		/// @param button Button to check.
		/// @return Whether it is presed.
		/// @warning Controller input is currently unimplemented!
		bool isButtonDown(String const& button);

		/// @brief Returns whether the given button has just been pressed.
		/// @param button Button to check.
		/// @return Whether it has just been presed.
		/// @warning Controller input is currently unimplemented!
		bool isButtonJustPressed(String const& button);

		/// @brief Returns whether the given button has just been released.
		/// @param button Button to check.
		/// @return Whether it has just been released.
		/// @warning Controller input is currently unimplemented!
		bool isButtonJustReleased(String const& button);

		/// @brief Returns if the button is held (state > threshold).
		/// @param button Button to check.
		/// @return Whether it is currently being held.
		/// @warning Controller input is currently unimplemented!
		bool isButtonHeld(String const& button);

		/// @brief Returns if the button's state has changed.
		/// @param button Button to check.
		/// @return Whether its state has changed.
		/// @warning Controller input is currently unimplemented!
		bool hasButtonChanged(String const& button);

		/// @brief Returns whether there is a button binding with the given name.
		/// @param name Button bind name to check.
		/// @return Whether binding exists.
		bool isBound(String const& name);

		/// @brief Returns the bindings the given button is associated with.
		/// @param button Button to check.
		/// @return Button binds associated with the button.
		StringList getNamesForButton(Button const& button);

		/// @brief Button bind map.
		inline static ButtonMap binds;

		/// @brief Whether input is enabled.
		static inline bool enabled = true;

		/// @brief Input mask.
		inline static InputMask mask = InputMask::IM_ALL;

		/// @brief How long before the input is considered "held".
		inline static usize threshold = 2048;

		/// @brief 
		///		Refreshes the mouse capture state.
		///		Not required to call every cycle, as the program already handles it.
		static void refreshCapture();

		/// @brief Sets the manager's input capture window.
		/// @param target Window to register inputs from.
		static void setTargetWindow(Extern::Resource const& target) {
			if (target && !window)
				window = target;
		}

		/// @brief Clears the manager's input capture window.
		/// @param target Window to disconnect. Must match current target window.
		static void clearTargetWindow(Extern::Resource const& target) {
			if (target && window == target)
				window = nullptr;
		}

		/// @brief Returns whether a given window is the current target window.
		/// @param target Window to check.
		/// @return Whether it is the current target window.
		static bool isTargetWindow(Extern::Resource const& target) {
			return target && window == target;
		}

	private:
		usize getKeyState(KeyCode const& button);
		usize getJoyState(JoyCode const& button);
		usize getMouseState(MouseCode const& button);

		usize getLastKeyState(KeyCode const& button);
		usize getLastJoyState(JoyCode const& button);
		usize getLastMouseState(MouseCode const& button);

		usize getLastButtonState(Button const& button);

		/// @brief Whether mouse is captured.
		inline static bool mouseCaptured	= false;
		/// @brief Whether mouse is visible.
		inline static bool mouseVisible		= true;

		/// @brief Window to capture inputs from.
		inline static Extern::Resource window = nullptr;

		/// @brief Current keyboard state.
		inline static KeyBuffer		buffer;
		/// @brief Previous keyboard state.
		inline static KeyBuffer		last;
		/// @brief Mouse buffer state.
		inline static MouseBuffer	mouse;
	};
}

#endif // MAKAI_CORE_INPUT_MANAGER_H
