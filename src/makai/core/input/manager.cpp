#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include "manager.hpp"

#define sdlWindow ((SDL_Window*)window)

using namespace Makai; using namespace Makai::Input;

SDL_Event pollEvents() {
	SDL_Event ev;
	SDL_PollEvent(&ev);
	return ev;
}

constexpr SDL_Scancode convert(KeyCode const& key) {
	int16 code = (int16)(key);
	++code;
	if (key >= KeyCode::KC_A)				code += 3;
	if (key >= KeyCode::KC_KEYPAD_COMMA)	code += 4;
	if (key >= KeyCode::KC_KEYPAD_00)		code += 11;
	if (key >= KeyCode::KC_LEFT_CTRL)		code += 2;
	if (key >= KeyCode::KC_MODE)			code += 25;
	return (SDL_Scancode)code;
}

constexpr KeyCode convert(SDL_Scancode const& key) {
	int16 code = (int16)(key);
	--code;
	if (key >= SDL_SCANCODE_A)			code -= 3;
	if (key >= SDL_SCANCODE_KP_COMMA)	code -= 4;
	if (key >= SDL_SCANCODE_KP_00)		code -= 11;
	if (key >= SDL_SCANCODE_LCTRL)		code -= 2;
	if (key >= SDL_SCANCODE_MODE)		code -= 25;
	return (KeyCode)code;
}

constexpr bool isValidKey(SDL_Scancode const& code) {
	int16 value = (int16)(code);
	// This is awful.
	if (value < 4)					return false;
	if (value > 129 && value < 133)	return false;
	if (value > 164 && value < 176)	return false;
	if (value > 221 && value < 224)	return false;
	if (value > 221 && value < 224)	return false;
	if (value > 231 && value < 257)	return false;
	return true;
}

#define RECIPROCATE(KC, SC) static_assert((convert(KC) == (SC))); static_assert((convert(SC) == (KC)))

RECIPROCATE(KeyCode::KC_UNKNOWN,		SDL_SCANCODE_UNKNOWN	);
RECIPROCATE(KeyCode::KC_A,				SDL_SCANCODE_A			);
RECIPROCATE(KeyCode::KC_KEYPAD_COMMA,	SDL_SCANCODE_KP_COMMA	);
RECIPROCATE(KeyCode::KC_KEYPAD_00,		SDL_SCANCODE_KP_00		);
RECIPROCATE(KeyCode::KC_LEFT_CTRL,		SDL_SCANCODE_LCTRL		);
RECIPROCATE(KeyCode::KC_MODE,			SDL_SCANCODE_MODE		);

RECIPROCATE(KeyCode::KC_UP,		SDL_SCANCODE_UP		);
RECIPROCATE(KeyCode::KC_DOWN,	SDL_SCANCODE_DOWN	);
RECIPROCATE(KeyCode::KC_LEFT,	SDL_SCANCODE_LEFT	);
RECIPROCATE(KeyCode::KC_RIGHT,	SDL_SCANCODE_RIGHT	);

constexpr SDL_GameControllerButton convert(JoyCode const& btn) {
	return (SDL_GameControllerButton)btn;
}

constexpr JoyCode convert(SDL_GameControllerButton const& btn) {
	return (JoyCode)btn;
}

void Manager::update() {
	// Get keyboard state
	int keyCount = 0;
	const unsigned char* state = SDL_GetKeyboardState(&keyCount);
	for (int i = 0; i < keyCount; i++) {
		// Jankify
		SDL_Scancode	button	= (SDL_Scancode)(i);
		KeyCode			code	= convert(button);
		if (!isValidKey(button)) continue;
		// Get previous key state
		usize buttonState = 0;
		if (buffer[code]) buttonState = buffer[code];
		// If button is pressed...
		if(state[button]) {
			// If buffer not overflowing, increment buffer
			if(buttonState < 0xFFFF) buttonState++;
		}
		// Else, zero state
		else buttonState = 0;
		// Copy previous state to secondary buffer
		last[code]		= buffer[code];
		// Set buffer to button state
		buffer[code]	= buttonState;
	}
	// Get mouse structures
	MouseState& lm = mouse.local;
	MouseState& gm = mouse.global;
	MouseState& rm = mouse.relative;
	// Save last positional data
	lm.last = lm.pos;
	gm.last = gm.pos;
	rm.last = rm.pos;
	// Get position & button data
	uint32 btn = SDL_GetMouseState(&lm.pos.x, &lm.pos.y);
	SDL_GetGlobalMouseState(&gm.pos.x, &gm.pos.y);
	SDL_GetRelativeMouseState(&rm.pos.x, &rm.pos.y);
	// Update button data
	for (auto i = 0; i < (((int)MouseCode::MC_MAX_BUTTONS)-1); i++) {
		// Jankify
		MouseCode code		= (MouseCode)i;
		// Get previous button state
		usize buttonState = 0;
		if (mouse.buffer[code]) buttonState = mouse.buffer[code];
		// If button is pressed.
		if(btn & SDL_BUTTON(i)) {
			// If buffer not overflowing, increment buffer
			if(buttonState < 0xFFFF) buttonState++;
		}
		// Else, zero state
		else buttonState = 0;
		// Copy previous state to secondary buffer
		mouse.last[code]	= mouse.buffer[code];
		// Set buffer to button state
		mouse.buffer[code]	= buttonState;
	}
}

usize Manager::getKeyState(KeyCode const& button) {
	if (
		!enabled
	||	!((ubyte)mask & (ubyte)InputMask::IM_KEY)
	||	button == KeyCode::KC_UNKNOWN
	) return 0;
	return buffer[button];
}

usize Manager::getMouseState(MouseCode const& button) {
	if (
		!enabled
	||	!((ubyte)mask & (ubyte)InputMask::IM_MOUSE)
	||	button == MouseCode::MC_UNKNOWN
	) return 0;
	return mouse.buffer[button];
}

usize Manager::getJoyState(JoyCode const& button) {
	if (
		!enabled
	||	!((ubyte)mask & (ubyte)InputMask::IM_JOY)
	||	button == JoyCode::JC_UNKNOWN
	) return 0;
	return 0;
}

usize Manager::getLastKeyState(KeyCode const& button) {
	if (
		!enabled
	||	!((ubyte)mask & (ubyte)InputMask::IM_KEY)
	||	button == KeyCode::KC_UNKNOWN
	) return 0;
	return last[button];
}

usize Manager::getLastMouseState(MouseCode const& button) {
	if (
		!enabled
	||	!((ubyte)mask & (ubyte)InputMask::IM_MOUSE)
	||	button == MouseCode::MC_UNKNOWN
	) return 0;
	return mouse.last[button];
}

usize Manager::getLastJoyState(JoyCode const& button) {
	if (
		!enabled
	||	!((ubyte)mask & (ubyte)InputMask::IM_JOY)
	||	button == JoyCode::JC_UNKNOWN
	) return 0;
	return 0;
}

constexpr bool isStateDown(usize const state) {
	return state > 0;
}

constexpr bool isStateJustPressed(usize const state) {
	return state == 1;
}

constexpr bool isStateJustReleased(usize const state, bool const justChanged) {
	return (justChanged && (state == 0));
}

constexpr bool isStateHeld(usize const state, usize const threshold) {
	return state > threshold;
}

constexpr bool hasStateChanged(usize const state, usize const last) {
	return state != last;
}
/*
/// Returns the button that was most recently pressed.
KeyboardButton mostRecentKeyboardButtonDown() {
	KeyboardButton button	= SDL_SCANCODE_UNKNOWN;
	usize duration	= Math::Max::UINT_V;
	for (auto [b, d] : buffer)
		if ((d) && d < duration) {
			button		= b;
			duration	= d;
		}
	return button;
}

/// Returns the button that was most recently just pressed.
KeyboardButton mostRecentKeyboardButtonJustPressed() {
	for (auto [b, d] : buffer)
		if (d == 1)
			return b;
	return SDL_SCANCODE_UNKNOWN;
}

/// Returns the button that was most recently held.
KeyboardButton mostRecentKeyboardButtonHeld() {
	KeyboardButton button	= SDL_SCANCODE_UNKNOWN;
	usize duration	= Math::Max::UINT_V;
	for (auto [b, d] : buffer)
		if (d > threshold && d < duration) {
			button		= b;
			duration	= d;
		}
	return button;
}

/// Returns the button that was most recently changed.
KeyboardButton mostRecentKeyboardButtonChanged() {
	for (auto [b, d] : buffer)
		if (d != last[b])
			return b;
	return SDL_SCANCODE_UNKNOWN;
}

/// Returns the button that was most recently just released.
KeyboardButton mostRecentKeyboardButtonJustReleased() {
	for (auto [b, d] : buffer)
		if (d != last[b] && d == 0)
			return b;
	return SDL_SCANCODE_UNKNOWN;
}

/// Returns all buttons currently pressed.
List<KeyboardButton> getKeyboardButtonsDown() {
	List<KeyboardButton> buttons;
	buttons.reserve(buffer.size());
	for (auto [b, d] : buffer)
		if (isButtonDown(b))
			buttons.pushBack(b);
	return buttons;
}

/// Returns all buttons currently just pressed.
List<KeyboardButton> getKeyboardButtonsJustPressed() {
	List<KeyboardButton> buttons;
	buttons.reserve(buffer.size());
	for (auto [b, d] : buffer)
		if (isButtonJustPressed(b))
			buttons.pushBack(b);
	return buttons;
}

/// Returns all buttons currently just released.
List<KeyboardButton> getKeyboardButtonsJustReleased() {
	List<KeyboardButton> buttons;
	buttons.reserve(buffer.size());
	for (auto [b, d] : buffer)
		if (isButtonJustReleased(b))
			buttons.pushBack(b);
	return buttons;
}

/// Returns all buttons whose state changed.
List<KeyboardButton> getKeyboardButtonsChanged() {
	List<KeyboardButton> buttons;
	for (auto [b, d] : buffer)
		if (isButtonChanged(b))
			buttons.pushBack(b);
	return buttons;
}

/// Returns all buttons currently held.
List<KeyboardButton> getKeyboardButtonsHeld() {
	List<KeyboardButton> buttons;
	buttons.reserve(buffer.size());
	for (auto [b, d] : buffer)
		if (isButtonHeld(b))
			buttons.pushBack(b);
	return buttons;
}
*/

usize Manager::getButtonState(Button const& button) {
	if (!enabled) return 0;
	switch (button.getType()) {
		case ButtonCodeType::BCT_KEY:	return getKeyState(button.getCode().key);
		case ButtonCodeType::BCT_MOUSE:	return getMouseState(button.getCode().mouse);
		case ButtonCodeType::BCT_JOY:	return getJoyState(button.getCode().joy);
	}
	return 0;
}

usize Manager::getLastButtonState(Button const& button) {
	if (!enabled) return 0;
	switch (button.getType()) {
		case ButtonCodeType::BCT_KEY:	return getLastKeyState(button.getCode().key);
		case ButtonCodeType::BCT_MOUSE:	return getLastMouseState(button.getCode().mouse);
		case ButtonCodeType::BCT_JOY:	return getLastJoyState(button.getCode().joy);
	}
	return 0;
}

bool Manager::isButtonDown(Button const& button) {
	if (!enabled) return false;
	return isStateDown(getButtonState(button));
}

bool Manager::isButtonJustPressed(Button const& button) {
	if (!enabled) return false;
	return isStateJustPressed(getButtonState(button));
}

bool Manager::isButtonJustReleased(Button const& button) {
	if (!enabled) return false;
	return isStateJustReleased(getButtonState(button), hasButtonChanged(button));
}

bool Manager::isButtonHeld(Button const& button) {
	if (!enabled) return false;
	return isStateHeld(getButtonState(button), threshold);
}

bool Manager::hasButtonChanged(Button const& button) {
	if (!enabled) return false;
	return hasStateChanged(getButtonState(button), getLastButtonState(button));
}

usize Manager::getButtonState(String const& button) {
	if (!enabled || !isBound(button))
		return 0;
	usize state = 0, current = 0;
	for (Button& btn: binds[button])
		if ((current = getButtonState(btn)) > state)
			state = current;
	return state;
}

bool Manager::isButtonDown(String const& button) {
	if (!enabled || !isBound(button)) return false;
	return isStateDown(getButtonState(button));
}

bool Manager::isButtonJustPressed(String const& button) {
	if (!enabled || !isBound(button)) return false;
	return isStateJustPressed(getButtonState(button));
}

bool Manager::isButtonJustReleased(String const& button) {
	if (!enabled || !isBound(button)) return false;
	return isStateJustReleased(getButtonState(button), hasButtonChanged(button));
}

bool Manager::isButtonHeld(String const& button) {
	if (!enabled || !isBound(button)) return false;
	return isStateHeld(getButtonState(button), threshold);
}

bool Manager::hasButtonChanged(String const& button) {
	if (!enabled || !isBound(button))
		return false;
	for (Button& btn: binds[button])
		if (hasButtonChanged(btn)) return true;
	return false;
}

bool Manager::isBound(String const& name) {
	return binds.contains(name);
}

StringList Manager::getNamesForButton(Button const& button) {
	StringList names;
	for (auto& [name, buttons]: binds) {
		for (Button& btn: buttons) {
			if (button == btn) {
				names.pushBack(name);
				break;
			}
		}
	}
	return names;
}

List<Button> Manager::getButtonsDown() {
	List<Button> buttons;
	buttons.reserve(buffer.size() + mouse.buffer.size());
	for (auto [b, d] : buffer)
		if (isButtonDown(b))
			buttons.pushBack(b);
	for (auto [b, d] : mouse.buffer)
		if (isButtonDown(b))
			buttons.pushBack(b);
	return buttons;
}

List<Button> Manager::getButtonsJustPressed() {
	List<Button> buttons;
	buttons.reserve(buffer.size() + mouse.buffer.size());
	for (auto [b, d] : buffer)
		if (isButtonJustPressed(b))
			buttons.pushBack(b);
	for (auto [b, d] : mouse.buffer)
		if (isButtonJustPressed(b))
			buttons.pushBack(b);
	return buttons;
}

List<Button> Manager::getButtonsJustReleased() {
	List<Button> buttons;
	buttons.reserve(buffer.size() + mouse.buffer.size());
	for (auto [b, d] : buffer)
		if (isButtonJustReleased(b))
			buttons.pushBack(b);
	for (auto [b, d] : mouse.buffer)
		if (isButtonJustReleased(b))
			buttons.pushBack(b);
	return buttons;
}

List<Button> Manager::getButtonsChanged() {
	List<Button> buttons;
	buttons.reserve(buffer.size() + mouse.buffer.size());
	for (auto [b, d] : buffer)
		if (hasButtonChanged(b))
			buttons.pushBack(b);
	for (auto [b, d] : mouse.buffer)
		if (hasButtonChanged(b))
			buttons.pushBack(b);
	return buttons;
}

List<Button> Manager::getButtonsHeld() {
	List<Button> buttons;
	buttons.reserve(buffer.size() + mouse.buffer.size());
	for (auto [b, d] : buffer)
		if (isButtonHeld(b))
			buttons.pushBack(b);
	for (auto [b, d] : mouse.buffer)
		if (isButtonHeld(b))
			buttons.pushBack(b);
	return buttons;
}

void Manager::refreshCapture() {
	DEBUGLN("Refreshing mouse capture states...");
	setMouseCapturing(mouseCaptured, !mouseVisible);
}

void Manager::setMouseCapturing(bool const enabled, bool const hideCursor) {
	if (!window) return;
	SDL_SetWindowGrab(sdlWindow, enabled ? SDL_TRUE : SDL_FALSE);
	SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
	setCursorVisibility(!hideCursor);
	mouseCaptured = enabled;
}

void Manager::setCursorVisibility(bool const visible) {
	if (!window) return;
	SDL_ShowCursor(enabled ? SDL_TRUE : SDL_FALSE);
	mouseVisible = enabled;
}

/// Returns the mouse position relative to the window.
Vector2 Manager::getWindowMousePosition() {
	return Vector2(mouse.local.pos.x, mouse.local.pos.y);
}

/// Returns the mouse position relative to the desktop.
Vector2 Manager::getDesktopMousePosition() {
	return Vector2(mouse.global.pos.x, mouse.global.pos.y);
}

/// Returns which direction the mouse is moving towards.
Vector2 Manager::getMouseDirection() {
	return Vector2(mouse.relative.pos.x, mouse.relative.pos.y);
}
