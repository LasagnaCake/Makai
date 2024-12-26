#include "sound.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

using namespace Makai; using namespace Makai::Audio;

namespace Extern = Makai::Extern;

#define sdlSource ((Mix_Chunk*)source)

void Sound::stopAll(usize const fade) {
	if (fade)
		Mix_FadeOutChannel(-1, fade);
	else
		Mix_HaltChannel(-1);
}

void Sound::pauseAll() {
	Mix_Pause(-1);
}

void Sound::resumeAll() {
	Mix_Resume(-1);
}

void Sound::setMasterVolume(schar const volume) {
	Mix_Volume(-1, volume);
}

schar Sound::getMasterVolume() {
	return Mix_Volume(-1, -1);
}

void Sound::play(
	int const	loops,
	usize const	fadeInTime,
	bool const	force
) {
	if (!exists() || delay > 0) return;
	if (active() && !force) return;
	if (fadeInTime)
		channel = Mix_FadeInChannel(active() ? channel : -1, sdlSource, loops, fadeInTime);
	else
		channel = Mix_PlayChannel(active() ? channel : -1, sdlSource, loops);
}

void Sound::playOnceThisFrame(
	int const	loops,
	usize const	fadeInTime,
	bool const	force
) {
	if (!exists() || delay > 0) return;
	if (active() && !force) return;
	if (!canPlayThisFrame) return;
	canPlayThisFrame = false;
	if (fadeInTime)
		channel = Mix_FadeInChannel(getChannel(), sdlSource, loops, fadeInTime);
	else
		channel = Mix_PlayChannel(getChannel(), sdlSource, loops);
}

void Sound::playOnceAndWait(
	int const	loops,
	usize const	fadeInTime,
	bool const	force,
	usize const	cycles
) {
	if (!exists() || delay > 0) return;
	if (active() && !force) return;
	if (!canPlayThisFrame) return;
	canPlayThisFrame = false;
	delay = cycles;
	if (fadeInTime)
		channel = Mix_FadeInChannel(getChannel(), sdlSource, loops, fadeInTime);
	else
		channel = Mix_PlayChannel(getChannel(), sdlSource, loops);
}

void Sound::stop(usize const fadeOutTime) {
	if (!exists()) return;
	if (!active()) return;
	if (fadeOutTime)
		Mix_FadeOutChannel(channel, fadeOutTime);
	else
		Mix_HaltChannel(channel);
}

void Sound::pause() {
	if (!exists()) return;
	if (!active()) return;
	Mix_Pause(channel);
}

void Sound::resume() {
	if (!exists()) return;
	if (!active()) return;
	Mix_Resume(channel);
}

void Sound::setVolume(uchar const volume) {
	if (!exists()) return;
	Mix_VolumeChunk(sdlSource, volume);
}

int Sound::getVolume() {
	if (!exists()) return -1;
	return Mix_VolumeChunk(sdlSource, -1);
}

void Sound::onCreate(Extern::Resource const& data) {
	if (!isOpen()) throw Error::FailedAction(
		"Failed to load file: Audio system is closed!",
		CTL_CPP_PRETTY_SOURCE
	);
	source = (Extern::Resource)Mix_LoadWAV_RW((SDL_RWops*)data, true);
	if (!source)
		throw Error::FailedAction(
			"Could not load file!",
			String(Mix_GetError()),
			CTL_CPP_PRETTY_SOURCE
		);
}

void Sound::onDestroy() {
	DEBUGLN("Destroying sound source...");
	if (isOpen()) {
		if (active())
			Mix_HaltChannel(channel);
		Mix_FreeChunk(sdlSource);
	}
	DEBUGLN("Sound source deleted!");
}

void Sound::onUpdate() {
	canPlayThisFrame = true;
	if (!active()) {
		channel = -1;
		return;
	}
	if (delay > 0) delay--;
}

int Sound::getChannel() {
	return active() ? channel : -1;
}

bool Sound::active() {
	if (!exists())		return false;
	if (channel == -1)	return false;
	return (
		Mix_Playing(channel)
	&&	Mix_GetChunk(channel) == sdlSource
	);
}
