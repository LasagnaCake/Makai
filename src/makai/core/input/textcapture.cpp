#include "textcapture.hpp"
#include "SDL2/SDL_stdinc.h"

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <windows.h>
#include <dwmapi.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#else
#include <SDL2/SDL.h>
#endif
#include <SDL2/SDL_keyboard.h>

using namespace Makai;
using namespace Input;

void TextCapture::onUpdate(String const& text) {
	buffer += text;
}

String TextCapture::value() const {
	return buffer;
}

void TextCapture::clear() {
	buffer.clear();
}

void TextCapture::begin() {
	SDL_StartTextInput();
}

void TextCapture::end() {
	SDL_StopTextInput();
}

bool TextCapture::capturing() {
	return SDL_IsTextInputActive() == SDL_TRUE;
}
