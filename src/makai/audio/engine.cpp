#include "engine.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <miniaudio.h>

using namespace Makai::Audio;
using namespace Makai;

struct Engine::Resource {
	ma_engine engine;

	Resource() {
		if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
			throw Error::FailedAction(
				"Failed to initialize audio engine!",
				CTL_CPP_PRETTY_SOURCE
			);
		}
	}

	~Resource() {
		ma_engine_uninit(&engine);
	}
};

struct Engine::Group::Resource {
	ma_sound_group		group;

	Instance<Resource>	parent;

	~Resource() {
		ma_sound_group_uninit(&group);
	}
};

struct Engine::Sound::Resource {
	ma_sound			source;
	ma_decoder			decoder;
	ma_decoder_config	config;

	BinaryData<>						data;
	Instance<Engine::Resource>			engine;
	Instance<Engine::Group::Resource>	group;

	~Resource() {
		ma_sound_uninit(&source);
		ma_decoder_uninit(&decoder);
	}
};

Engine::Engine()			{			}
Engine::~Engine()			{close();	}
Engine::Sound::Sound()		{			}
Engine::Sound::~Sound()		{			}
Engine::Group::Group()		{			}
Engine::Group::~Group()		{			}

void Engine::stopAllSounds() {
}

void Engine::open() {
	if (exists()) return;
	instance.bind(new Resource{});
}

void Engine::close() {
	if (!exists()) return;
	instance.unbind();
}

static inline ma_uint32 modeFlags(Engine::LoadMode const mode) {
	switch (mode) {
		case Engine::LoadMode::LM_STREAM:			return MA_SOUND_FLAG_STREAM;
		case Engine::LoadMode::LM_PRELOAD:			return MA_SOUND_FLAG_DECODE;
		case Engine::LoadMode::LM_PRELOAD_ASYNC:	return MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC;
	}
}

Instance<Engine::Group> Engine::createGroup(Handle<Engine::Group> const& parent) {
	if (!exists()) return nullptr;
	Instance<Group> group	= new Group();
	group->instance->parent	= parent->instance;
	if (
		ma_sound_group_init(
			&instance->engine,
			0,
			&parent->instance->group,
			&group->instance->group
		) != MA_SUCCESS
	) return nullptr;
}

Instance<Engine::Sound> Engine::createSound(BinaryData<> const& data, LoadMode const mode, Handle<Group> const& group) {
	if (!exists()) return nullptr;
	Instance<Sound> sound = new Sound();
	sound->instance->config = ma_decoder_config_init_default();
	sound->instance->engine = instance;
	sound->instance->group = group->instance;
	if (
		ma_decoder_init_memory(
			sound->instance->data.data(),
			sound->instance->data.size(),
			&sound->instance->config,
			&sound->instance->decoder
		) != MA_SUCCESS
	)
		return nullptr;
	if (
		ma_sound_init_from_data_source(
			&instance->engine,
			&sound->instance->decoder,
			modeFlags(mode),
			NULL,
			&sound->instance->source
		) != MA_SUCCESS
	) return nullptr;
	return sound;
}

Engine::Sound& Engine::Sound::start(bool const loop, usize const fadeIn) {
	if (!exists()) return *this;
	setLooping(loop);
	if (!fadeIn)
		play();
	else
		ma_sound_set_fade_in_milliseconds(&instance->source, 0, 1, fadeIn);
	return *this;
}

Engine::Sound& Engine::Sound::start() {
	if (!exists()) return *this;
	ma_sound_start(&instance->source);
	return *this;
}

Engine::Sound& Engine::Sound::play() {
	if (!exists()) return *this;
	return *this;
}

Engine::Sound& Engine::Sound::pause() {
	if (!exists()) return *this;
	return *this;
}

Engine::Sound& Engine::Sound::stop() {
	if (!exists()) return *this;
	ma_sound_stop(&instance->source);
	return *this;
}

Engine::Sound& Engine::Sound::stop(usize const fadeOut) {
	if (!exists()) return *this;
	ma_sound_set_fade_in_milliseconds(&instance->source, -1, 0, fadeOut);
	return *this;
}

void Engine::Sound::setLooping(bool const loop) {
	if (!exists()) return;
	ma_sound_set_looping(&instance->source, loop ? MA_TRUE : MA_FALSE);
}

bool Engine::Sound::looping() {
	if (!exists()) return false;
	return ma_sound_is_looping(&instance->source) == MA_TRUE;
}

void Engine::Sound::setVolume(float const volume) {
	if (!exists()) return;
	ma_sound_set_volume(&instance->source, volume);
}

float Engine::Sound::getVolume() const {
	if (!exists()) return 0;
	return ma_sound_get_volume(&instance->source);
}