#include "source.hpp"

#include "../file/get.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

using namespace Makai; using namespace Makai::Audio;

List<AudioCallback*> updateQueue;

ISource::ISource() {
	updateQueue.pushBack(&yield);
}

ISource::ISource(String const& path): ISource() {
	create(path);
}

ISource::~ISource() {
	DEBUGLN("Deleting playable object...");
	updateQueue.eraseLike(&yield);
	destroy();
	DEBUGLN("Object deleted!");
}

void ISource::create(String const& path) {
	if (created) return;
	data = Makai::File::getBinary(path);
	onCreate(SDL_RWFromConstMem(data.data(), data.size()));
	created = true;
};

void ISource::destroy() {
	if (!created) return;
	onDestroy();
	data.clear();
	created = false;
};

void ISource::update() {
	for(AudioCallback*& c : updateQueue) (*c)();
}

bool ISource::exists() {
	return created;
}
