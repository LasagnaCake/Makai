#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include "popup.hpp"

using namespace Makai;

int Popup::showDialog(
	String const& title,
	String const& text,
	StringList const& options,
	PopupType const& type
) {
	usize buttonCount = options.size();
	SDL_MessageBoxButtonData* buttons = new SDL_MessageBoxButtonData[options.size()];
	usize idx = buttonCount - 1;
	for (usize i = 0; i < options.size(); ++i) {
		buttons[i] = SDL_MessageBoxButtonData {
			0,
			(int)idx,
			options[idx].cstr()
		};
		--idx;
	}
	uint32 flags = 0;
	switch (type) {
		case Popup::PopupType::PT_ERROR:	flags = SDL_MESSAGEBOX_ERROR;		break;
		case Popup::PopupType::PT_INFO:		flags = SDL_MESSAGEBOX_INFORMATION;	break;
		case Popup::PopupType::PT_WARN:		flags = SDL_MESSAGEBOX_WARNING;		break;
	}
	const SDL_MessageBoxData messageBoxData = {
		flags,					/* .flags */
		NULL,					/* .window */
		title.cstr(),			/* .title */
		text.cstr(),			/* .message */
		(int)options.size(),	/* .numbuttons */
		buttons,				/* .buttons */
		NULL
	};
	int buttonid;
	if (SDL_ShowMessageBox(&messageBoxData, &buttonid) < 0) {
		throw Error::FailedAction(
			"Failed to show popup!",
			SDL_GetError(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	delete[] buttons;
	return buttonid;
}

void Popup::showError(String const& what) {
	//PlaySound("SystemExclamation", NULL, SND_ASYNC);
	Popup::showDialog(
		"Error!",
		what,
		Popup::Option::OK,
		Popup::PopupType::PT_ERROR
	);
}
