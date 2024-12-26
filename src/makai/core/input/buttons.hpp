#ifndef MAKAILIB_CORE_INPUT_BUTTONS_H
#define MAKAILIB_CORE_INPUT_BUTTONS_H

#include "../../compat/ctl.hpp"

/// @brief User input facilities.
namespace Makai::Input {
	/// @brief Keyboard key code.
	enum class KeyCode {
		// Unknown
		KC_UNKNOWN = -1,
		// Alphabet
		KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N,
		KC_O, KC_P, KC_Q, KC_R, KC_S, KC_T, KC_U, KC_V, KC_W, KC_X, KC_Y, KC_Z,
		// Numbers
		KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0,
		// Special keys 1
		KC_RETURN, KC_ESCAPE, KC_BACKSPACE, KC_TAB, KC_SPACE,
		// Math symbols 1
		KC_PLUS, KC_MINUS, KC_EQUALS,
		// Brackets
		KC_OPEN_BRACKET, KC_BRACKET_L = KC_OPEN_BRACKET, KC_CLOSE_BRACKET, KC_BRACKET_R = KC_CLOSE_BRACKET,
		// Assorted symbols 1
		KC_BACKSLASH,
		KC_ISO_HASH,
		KC_SEMICOLON, KC_APOSTROPHE, KC_GRAVE,
		KC_COMMA, KC_PERIOD, KC_SLASH, KC_FWDSLASH = KC_SLASH,
		// Caps Lock
		KC_CAPS_LOCK,
		// Function keys 1
		KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12,
		// Special keys 2
		KC_PRINT_SCREEN, KC_SCROLL_LOCK, KC_PAUSE, KC_INSERT, KC_HOME, KC_PAGE_UP, KC_DELETE, KC_PAGE_DOWN,
		// Directional Keys
		KC_RIGHT, KC_LEFT, KC_DOWN, KC_UP,
		// Num lock
		KC_NUM_LOCK,
		// Keypad keys 1
		KC_KEYPAD_DIV, KC_KEYPAD_MUL, KC_KEYPAD_MINUS, KC_KEYPAD_PLUS, KC_KEYPAD_ENTER,
		KC_KEYPAD_1, KC_KEYPAD_2, KC_KEYPAD_3, KC_KEYPAD_4, KC_KEYPAD_5,
		KC_KEYPAD_6, KC_KEYPAD_7, KC_KEYPAD_8, KC_KEYPAD_9, KC_KEYPAD_0,
		KC_KEYPAD_PERIOD,
		// ISO key
		KC_ISO_BACKSLASH,
		// Special keys 2
		KC_APP, KC_POWER,
		// Keypad equals
		KC_KEYPAD_EQUALS,
		// Function keys 2
		KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
		// Special keys 3
		KC_HELP, KC_MENU,
		KC_SELECT, KC_STOP, KC_AGAIN, KC_UNDO, KC_CUT, KC_COPY, KC_PASTE, KC_FIND,
		KC_MUTE, KC_VOLUME_UP, KC_VOLUME_DOWN,
		// Keypad keys 2
		KC_KEYPAD_COMMA,
		KC_KEYPAD_EQUALS_AS400,
		// World (international) keys
		KC_WORLD_1, KC_WORLD_2, KC_WORLD_3, KC_YEN = KC_WORLD_3, KC_WORLD_4, KC_WORLD_5,
		KC_WORLD_6, KC_WORLD_7, KC_WORLD_8, KC_WORLD_9,
		// Language keys
		KC_LANG_1, KC_ENGLISH = KC_LANG_1, KC_HANGUL = KC_LANG_1,
		KC_LANG_2, KC_HANJA = KC_LANG_2,
		KC_LANG_3, KC_KATAKANA = KC_LANG_3,
		KC_LANG_4, KC_HIRAGANA = KC_LANG_4,
		KC_LANG_5, KC_ZENKAKU = KC_LANG_5, KC_HANKAKU = KC_LANG_5,
		KC_LANG_6, KC_LANG_7, KC_LANG_8, KC_LANG_9,
		// Special keys 4
		KC_ALT_ERASE, KC_SYS_REQ, KC_AC_CANCEL, KC_CLEAR, KC_PRIOR, KC_RETURN_2, KC_SEPARATOR,
		KC_OUT, KC_OPER, KC_CLEAR_AGAIN,
		// Special selection keys
		KC_CURSOR_SELECT, KC_CUR_SEL = KC_CURSOR_SELECT,
		KC_EXTENDED_SELECT, KC_EX_SEL = KC_EXTENDED_SELECT,
		// Keypad keys 3
		KC_KEYPAD_00, KC_KEYPAD_000,
		// Unit characters
		KC_THOUSAND_SEP, KC_DECIMAL_SEP, KC_CURRENCY, KC_SUB_CURRENCY,
		// Keypad keys 4
		KC_KEYPAD_OPEN_PAREN, KC_KEYPAD_PAREN_L = KC_KEYPAD_OPEN_PAREN,
		KC_KEYPAD_CLOSE_PAREN, KC_KEYPAD_PAREN_R = KC_KEYPAD_CLOSE_PAREN,
		KC_KEYPAD_OPEN_BRACE, KC_KEYPAD_BRACE_L = KC_KEYPAD_OPEN_BRACE,
		KC_KEYPAD_CLOSE_BRACE, KC_KEYPAD_BRACE_R = KC_KEYPAD_CLOSE_BRACE,
		KC_KEYPAD_TAB, KC_KEYPAD_BACKSPACE,
		KC_KEYPAD_A, KC_KEYPAD_B, KC_KEYPAD_C, KC_KEYPAD_D, KC_KEYPAD_E, KC_KEYPAD_F,
		KC_KEYPAD_XOR, KC_KEYPAD_POWER, KC_KEYPAD_PCT, KC_KEYPAD_LT, KC_KEYPAD_GT,
		KC_KEYPAD_AMP, KC_KEYPAD_DOUBLE_AMP,
		KC_KEYPAD_VBAR, KC_KEYPAD_DOUBLE_VBAR,
		KC_KEYPAD_COLOR, KC_KEYPAD_HASH, KC_KEYPAD_POUND = KC_KEYPAD_HASH,
		KC_KEYPAD_SPACE, KC_KEYPAD_AT, KC_KEYPAD_EXCLAMATION,
		KC_KEYPAD_MEM_STORE, KC_KEYPAD_MEM_RECALL, KC_KEYPAD_MEM_RCL = KC_KEYPAD_MEM_RECALL,
		KC_KEYPAD_MEM_CLEAR, KC_KEYPAD_MEM_ADD, KC_KEYPAD_MEM_SUB, KC_KEYPAD_MEM_MUL, KC_KEYPAD_MEM_DIV,
		KC_KEYPAD_PLUS_MINUS, KC_KEYPAD_CLEAR, KC_KEYPAD_CLEAR_ENTRY,
		KC_KEYPAD_BINARY, KC_KEYPAD_BIN = KC_KEYPAD_BINARY, KC_KEYPAD_0B = KC_KEYPAD_BINARY,
		KC_KEYPAD_OCTAL, KC_KEYPAD_OCT = KC_KEYPAD_OCTAL, KC_KEYPAD_0O = KC_KEYPAD_OCTAL,
		KC_KEYPAD_DECIMAL, KC_KEYPAD_DEC = KC_KEYPAD_DECIMAL, KC_KEYPAD_0D = KC_KEYPAD_DECIMAL,
		KC_KEYPAD_KEXADECIMAL, KC_KEYPAD_HEX = KC_KEYPAD_KEXADECIMAL, KC_KEYPAD_0X = KC_KEYPAD_KEXADECIMAL,
		// Special keys 5
		KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_LEFT_ALT, KC_LEFT_OS,
		KC_RIGHT_CTRL, KC_RIGHT_SHIFT, KC_RIGHT_ALT, KC_ALT_GR = KC_RIGHT_ALT, KC_RIGHT_OS,
		KC_MODE,
		// Media keys
		KC_MEDIA_NEXT, KC_MEDIA_PREVIOUS, KC_MEDIA_PREV = KC_MEDIA_PREVIOUS,
		KC_MEDIA_STOP, KC_MEDIA_PLAY, KC_MEDIA_MUTE, KC_MEDIA_SELECT, KC_MEDIA_SEL = KC_MEDIA_SELECT,
		// Weird keys
		KC_AL_BROWSER, KC_AL_WEB = KC_AL_BROWSER,
		KC_MAIL,
		KC_AC_SEARCH, KC_AC_HOME, KC_AC_BACK, KC_AC_FORWARD, KC_AC_STOP, KC_AC_REFRESH, KC_AC_BOOKMARKS,
		// Walther keys
		KC_BRIGHTNESS_DOWN, KC_BRIGHT_DOWN = KC_BRIGHTNESS_DOWN,
		KC_BRIGHTNESS_UP, KC_BRIGHT_UP = KC_BRIGHTNESS_UP,
		KC_DISPLAY_SWITCH, KC_DISP_SWITCH = KC_DISPLAY_SWITCH,
		KC_KEYBOARD_LIGHTS_TOGGLE, KC_KB_LIGHTS_TOGGLE = KC_KEYBOARD_LIGHTS_TOGGLE,
		KC_KEYBOARD_LIGHTS_UP, KC_KB_LIGHTS_UP = KC_KEYBOARD_LIGHTS_UP,
		KC_KEYBOARD_LIGHTS_DOWN, KC_KB_LIGHTS_DOWN = KC_KEYBOARD_LIGHTS_DOWN,
		KC_EJECT, KC_SC_SLEEP,
		KC_APP_1, KC_APP_2,
		// Media keys 2
		KC_MEDIA_REWIND, KC_MEDIA_REV = KC_MEDIA_REWIND,
		KC_MEDIA_FAST_FORWARD, KC_MEDIA_FWD = KC_MEDIA_FAST_FORWARD,
		// Mobile keys
		KC_SOFT_LEFT, KC_SOFT_RIGHT, KC_CALL, KC_END_CALL,
		KC_MAX_KEYS = 512
	};

	/// @brief Controller button code.
	enum class JoyCode {
		// Unknown
		JC_UNKNOWN = -1,
		// Face buttons
		JC_A, JC_B, JC_X, JC_Y,
		// Menu buttons
		JC_BACK, JC_GUIDE, JC_START,
		// Stick buttons
		JC_LEFT_STICK, JC_RIGHT_STICK,
		// Shoulders
		JC_LEFT_SHOULDER, JC_RIGHT_SHOULDER,
		// D-Pad
		JC_DPAD_UP, JC_DPAD_DOWN, JC_DPAD_LEFT, JC_DPAD_RIGHT,
		// Extra button
		JC_EXTRA,
		// XBOX Paddles
		JC_XBOX_PADDLE_1, JC_XBOX_PADDLE_2, JC_XBOX_PADDLE_3, JC_XBOX_PADDLE_4,
		// PS[4/5] Touchpad
		JC_PS_TOUCH,
		// Max button count
		JC_MAX_BUTTONS
	};

	/// @brief Mouse button code.
	enum class MouseCode {
		MC_UNKNOWN = -1,
		MC_LEFT,
		MC_MIDDLE,
		MC_RIGHT,
		MC_MAX_BUTTONS
	};

	/// @brief Button code union.
	union ButtonCode {
		KeyCode		key;
		MouseCode	mouse;
		JoyCode		joy;
		usize		value;
	};

	/// @brief Button code type.
	enum class ButtonCodeType {
		BCT_KEY,
		BCT_MOUSE,
		BCT_JOY
	};

	/// @brief Button data structure.
	struct ButtonData {
		/// @brief Button code.
		ButtonCode		code;
		/// @brief Button type.
		ButtonCodeType	type;
	};

	/// @brief Input button.
	struct Button: private ButtonData {
		/// @brief Constructs the button from a keyboard key code.
		constexpr Button(KeyCode const& _code)		{code.key = _code;		type = ButtonCodeType::BCT_KEY;		}
		/// @brief Constructs the button from a mouse button code.
		constexpr Button(MouseCode const& _code)	{code.mouse = _code;	type = ButtonCodeType::BCT_MOUSE;	}
		/// @brief Constructs the button from a controller button code.
		constexpr Button(JoyCode const& _code)		{code.joy = _code;		type = ButtonCodeType::BCT_JOY;		}

		/// @brief Returns the button code.
		/// @return Button code.
		constexpr ButtonCode getCode() const 		{return code;	}
		/// @brief Returns the button type.
		/// @return Button tyoe.
		constexpr ButtonCodeType getType() const 	{return type;	}
		/// @brief Returns the button data.
		/// @return Button data.
		constexpr ButtonData data() const 			{return *this;	}

		/// @brief Equality comparison operator.
		/// @param other `Button` to compare with.
		/// @return Whether they're equal.
		constexpr bool operator==(Button const& other) const {
			return type == other.type && code.value == other.code.value;
		}
	private:
		friend class InputManager;
	};

	/// @brief Button list.
	using ButtonList = List<Button>;

	/// @brief Button bind map.
	using ButtonMap = Map<String, ButtonList>;
}

#endif // MAKAILIB_CORE_INPUT_BUTTONS_H
