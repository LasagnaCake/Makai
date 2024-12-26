#include "core.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

bool audioOpen = false;

void Makai::Audio::stopAll() {
	Mix_HaltMusic();
	Mix_HaltChannel(-1);
}

using Format	= Makai::Audio::Format;
using Formats	= Makai::Audio::Formats;

int getFlags(Formats const& formats) {
	if (formats.empty()) return 0;
	using enum Format;
	int flags = 0;
    for (Format f: formats) {
		switch(f) {
			using enum Format;
			case (AF_FLAC):		flags |= MIX_INIT_FLAC;		break;
			case (AF_MOD):		flags |= MIX_INIT_MOD;		break;
			case (AF_MP3):		flags |= MIX_INIT_MP3;		break;
			case (AF_OGG):		flags |= MIX_INIT_OGG;		break;
			case (AF_MIDI):		flags |= MIX_INIT_MID;		break;
			case (AF_OPUS):		flags |= MIX_INIT_OPUS;		break;
		//	case (AF_WAVPACK):	flags |= MIX_INIT_WAVPACK;	break;
			default: break;
		}
    }
    return flags;
}

void Makai::Audio::open(
	Formats const&	formats,
	uint const		channels,
	uint const		audioTracks
) {
	if (audioOpen) return;
	if (!Mix_Init(getFlags(formats))) {
		throw Error::FailedAction(
			"Unable to open audio system!",
			Mix_GetError(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	int result = Mix_OpenAudioDevice(
		48000,
		AUDIO_F32SYS,
		channels,
		1024,
		NULL,
		SDL_AUDIO_ALLOW_FORMAT_CHANGE
	|	SDL_AUDIO_ALLOW_SAMPLES_CHANGE
	);
	if (result) {
		throw Error::FailedAction(
			"Failed to open audio system!",
			Mix_GetError(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	Mix_AllocateChannels(audioTracks);
	audioOpen = true;
}

void Makai::Audio::restart(
	Formats const&	formats,
	uint const		channels,
	uint const		audioTracks
) {
	Makai::Audio::close();
	Makai::Audio::open(formats, channels, audioTracks);
}

void Makai::Audio::close() {
	if (!audioOpen) return;
	stopAll();
	Mix_CloseAudio();
	Mix_Quit();
	audioOpen = false;
}

bool Makai::Audio::isOpen() {
	return audioOpen;
}
