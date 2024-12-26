#include "music.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

using namespace Makai; using namespace Makai::Audio;

namespace Extern = Makai::Extern;

#define sdlSource ((Mix_Music*)source)

Mix_Music*				current	= nullptr;
AudioCallback const*	queued	= nullptr;

void Music::stop(usize const fade) {
	if (fade)
		Mix_FadeOutMusic(fade);
	else
		Mix_HaltMusic();
}

void Music::pause() {
	Mix_PauseMusic();
}

void Music::resume() {
	Mix_ResumeMusic();
}

void Music::setVolume(schar const volume) {
	Mix_VolumeMusic(volume);
}

schar Music::getVolume() {
	return Mix_VolumeMusic(-1);
}

Music::~Music() {
	//destroy();
}

void Music::play(int const loops, usize const fadeInTime) {
	if (!exists()) return;
	this->loops = loops;
	current	= sdlSource;
	if (fadeInTime)
		Mix_FadeInMusic(sdlSource, loops, fadeInTime);
	else
		Mix_PlayMusic(sdlSource, loops);
}

String Music::getTitle() {
	return String(Mix_GetMusicTitle(sdlSource));
}

Music::Metadata Music::getMetadata() {
	const char* data;
	return Music::Metadata {
		(data = Mix_GetMusicTitleTag(sdlSource)) ? data : "???",
		(data = Mix_GetMusicArtistTag(sdlSource)) ? data : "???",
		(data = Mix_GetMusicAlbumTag(sdlSource)) ? data : "???",
		(data = Mix_GetMusicCopyrightTag(sdlSource)) ? data : "???"
	};
}

void Music::switchInto(usize const fadeOutTime, usize const fadeInTime) {
	if (!exists()) return;
	this->fadeInTime = fadeInTime;
	queueMusic();
	stop(fadeOutTime);
}

void Music::switchInto(usize const fadeOutTime, usize const fadeInTime, int const loops) {
	if (!exists()) return;
	this->fadeInTime	= fadeInTime;
	this->loops			= loops;
	queueMusic();
	stop(fadeOutTime);
}

void Music::queueMusic() {
	if (!exists()) return;
	queued = &onQueue;
}

Extern::Resource Music::getSource() {
	return source;
}

void Music::onCreate(Extern::Resource const& data) {
	if (!isOpen()) throw Error::FailedAction(
		"Failed to load file: Audio system is closed!",
		CTL_CPP_PRETTY_SOURCE
	);
	source = (Extern::Resource)Mix_LoadMUS_RW((SDL_RWops*)data, true);
	if (!source)
		throw Error::FailedAction(
			"Could not load file!",
			String(Mix_GetError()),
			CTL_CPP_PRETTY_SOURCE
		);
}

void Music::onDestroy() {
	DEBUGLN("Deleting music source...");
	if (isOpen()) {
		if (queued == &onQueue)
			queued = nullptr;
		if (Mix_PlayingMusic() && current == sdlSource)
			Mix_HaltMusic();
		Mix_FreeMusic(sdlSource);
	}
	DEBUGLN("Music source deleted!");
}

void Music::onUpdate() {
	if (current != source) return;
}

void Music::update() {
	if (!Mix_PlayingMusic()) {
		current = nullptr;
		if (queued) {
			(*queued)();
			queued = nullptr;
		}
	}
}
