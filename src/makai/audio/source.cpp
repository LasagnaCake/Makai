#include "source.hpp"

#include "../file/get.hpp"
#include "core.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

using namespace Makai; using namespace Makai::Audio;

struct Audio::Source::Content {
	BinaryData<>		file;
	owner<Mix_Chunk>	source	= nullptr;
	SourceType			type	= SourceType::ST_SOUND;
	ssize				channel	= -1;

	~Content() {
		if (!isOpen()) return;
		Mix_FreeChunk(source);
	}

	bool active() {
		if (!source)		return false;
		if (channel == -1)	return false;
		return (
			Mix_Playing(channel)
		&&	Mix_GetChunk(channel) == source
		);
	}
};

Source::Source(): APeriodicAudio() {
}

Source::Source(String const& path, SourceType const type): APlayable() {
	create(path, type);
}

Source::~Source() {
	DEBUGLN("Deleting audio source object...");
	destroy();
	DEBUGLN("Object deleted!");
}

void Source::create(String const& path, SourceType const type) {
	if (created) return;
	if (!isOpen()) throw Error::FailedAction(
		"Failed to load file: Audio system is closed!",
		CTL_CPP_PRETTY_SOURCE
	);
	data.bind(new Source::Content{});
	data->file = Makai::File::getBinary(path);
	data->source = Mix_LoadWAV_RW(SDL_RWFromConstMem(data->file.data(), data->file.size()), true);
	data->type = type;
	if (!data->source)
		throw Error::FailedAction(
			"Could not load audio file [", path, "]!",
			String(Mix_GetError()),
			CTL_CPP_PRETTY_SOURCE
		);
	created = true;
};

void Source::setType(SourceType const type) {
	if (data) data->type = type;
};

void Source::destroy() {
	if (!created) return;
	source.unbind();
	created = false;
};

bool Source::exists() {
	return created;
}
