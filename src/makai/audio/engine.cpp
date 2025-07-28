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

	List<Instance<Sound::Resource>> sounds;
	List<Instance<Group::Resource>>	groups;

	Instance<Sound> createSound(BinaryData<> const& data, SoundType const mode, Handle<Group::Resource> const& group);
	Instance<Group> createGroup(Handle<Group::Resource> const& parent);

	Resource();

	~Resource();
};
	
using APeriodicGroup = APeriodic<Engine::Group::Resource>;

using APeriodicSound = APeriodic<Engine::Sound::Resource>;

struct Engine::Sound::Resource: APeriodicSound {
	ma_sound			source;
	ma_decoder			decoder;
	ma_decoder_config	config;

	BinaryData<>					data;
	Handle<Engine::Resource>		engine;
	Handle<Engine::Group::Resource>	group;

	SoundType type;
	
	usize cooldown = 0;

	bool paused = false;

	void update() {
		if (cooldown) --cooldown;
	}

	bool canPlayAgain() const {return cooldown;}

	usize toPCMFrames(float const time) const {
		return (ma_engine_get_sample_rate(ma_sound_get_engine(&source)) * time);
	}

	float toSeconds(usize const time) const {
		return (static_cast<float>(time) / ma_engine_get_sample_rate(ma_sound_get_engine(&source)));
	}

	~Resource();
};

struct Engine::Group::Resource: APeriodicGroup {
	ma_sound_group		group;

	Handle<Engine::Resource>	engine;
	Handle<Resource>			parent;
	List<Instance<Resource>>	children;

	void update() {}

	~Resource();
};

Engine::Resource::Resource()  {
	if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
		throw Error::FailedAction(
			"Failed to initialize audio engine!",
			CTL_CPP_PRETTY_SOURCE
		);
	}
}

Engine::Resource::~Resource() {
	ma_engine_uninit(&engine);
}

Engine::Sound::Resource::~Resource() {
	ma_sound_uninit(&source);
	ma_decoder_uninit(&decoder);
}

Engine::Group::Resource::~Resource() {
	ma_sound_group_uninit(&group);
}

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

static inline ma_uint32 modeFlags(Engine::SoundType const mode) {
	switch (mode) {
		case Engine::SoundType::EST_STREAMED:			return MA_SOUND_FLAG_STREAM;
		case Engine::SoundType::EST_PRELOADED:			return MA_SOUND_FLAG_DECODE;
		case Engine::SoundType::EST_PRELOADED_ASYNC:	return MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC;
	}
}

Instance<Engine::Group> Engine::Resource::createGroup(Handle<Group::Resource> const& parent) {
	Instance<Group> group = new Group();
	group->instance->engine = this;
	if (
		ma_sound_group_init(
			&engine,
			0,
			parent ? &parent->group : NULL,
			&group->instance->group
		) != MA_SUCCESS
	) return nullptr;
	if (parent) {
		group->instance->parent	= parent;
		parent->children.pushBack(group->instance);
	}
	return group;
}

Instance<Engine::Group> Engine::createGroup(Handle<Group> const& parent) {
	if (!exists()) return nullptr;
	return instance->createGroup(parent->instance);
}

Instance<Engine::Sound> Engine::Resource::createSound(
	BinaryData<> const& data,
	SoundType const type,
	Handle<Group::Resource> const& group
) {
	Instance<Sound> sound = new Sound();
	sound->instance->config = ma_decoder_config_init_default();
	sound->instance->engine = this;
	sound->instance->group = group;
	sound->instance->type = type;
	if (
		ma_decoder_init_memory(
			sound->instance->data.data(),
			sound->instance->data.size(),
			&sound->instance->config,
			&sound->instance->decoder
		) != MA_SUCCESS
	) return nullptr;
	if (
		ma_sound_init_from_data_source(
			&engine,
			&sound->instance->decoder,
			modeFlags(type) | MA_SOUND_FLAG_NO_SPATIALIZATION,
			NULL,
			&sound->instance->source
		) != MA_SUCCESS
	) return nullptr;
	sounds.pushBack(sound->instance);
	return sound;
}

Instance<Engine::Sound> Engine::createSound(BinaryData<> const& data, SoundType const type, Handle<Group> const& group) {
	if (!exists()) return nullptr;
	return instance->createSound(data, type, group->instance);
}

template<class T>
static bool oneInstanceFilter(Instance<typename T::Resource> const& inst) {
	return inst.count() < 2;
}

void Engine::onUpdate() {
	for (auto& group: instance->groups)
		if (group)
			group->update();
	for (auto& sound: instance->sounds)
		if (sound)
			sound->update();
	instance->groups.filter(oneInstanceFilter<Group>);
	instance->sounds.filter(oneInstanceFilter<Sound>);
}
	

Engine::Sound& Engine::Sound::play(bool const force, bool const loop, float const fadeInTime, usize const cooldown) {
	if (!exists()) return *this;
	if (!instance->canPlayAgain()) return * this;
	if (playing()) {
		if (!force) return *this;
		stop();
	}
	setLooping(loop);
	if (fadeInTime) {
		setVolume(0);
		fadeIn(fadeInTime);
	}
	instance->paused = false;
	setPlaybackTime(0);
	ma_sound_start(&instance->source);
	instance->cooldown = cooldown;
	return *this;
}

Engine::Sound& Engine::Sound::pause() {
	if (!exists() || paused()) return *this;
	ma_sound_start(&instance->source);
	instance->paused = true;
	return *this;
}

Engine::Sound& Engine::Sound::unpause() {
	if (!exists() || !paused()) return *this;
	ma_sound_stop(&instance->source);
	instance->paused = false;
	return *this;
}

Engine::Sound& Engine::Sound::stop(float const fadeOutTime) {
	if (!exists()) return *this;
	if (fadeOutTime)
		ma_sound_stop_with_fade_in_pcm_frames(&instance->source, instance->toPCMFrames(fadeOutTime));
	else ma_sound_stop(&instance->source);
	return *this;
}

Engine::Sound& Engine::Sound::setLooping(bool const loop) {
	if (!exists()) return *this;
	ma_sound_set_looping(&instance->source, loop ? MA_TRUE : MA_FALSE);
	return *this;
}

Engine::Sound& Engine::Sound::setPlaybackTime(float const time) {
	if (!exists()) return *this;
	ma_sound_seek_to_pcm_frame(&instance->source, instance->toPCMFrames(time));
	return *this;
}

float Engine::Sound::getPlaybackTime() const {
	if (!exists()) return 0;
	return instance->toSeconds(ma_sound_get_time_in_pcm_frames(&instance->source));
}

bool Engine::Sound::looping() const {
	if (!exists()) return false;
	return ma_sound_is_looping(&instance->source) == MA_TRUE;
}

bool Engine::Sound::playing() const {
	if (!exists()) return false;
	return ma_sound_is_playing(&instance->source) == MA_TRUE;
}

bool Engine::Sound::paused() const {
	if (!exists()) return false;
	return instance->paused;
}

Engine::Sound& Engine::Sound::setVolume(float const volume) {
	if (!exists()) return *this;
	ma_sound_set_volume(&instance->source, volume);
	return *this;
}

float Engine::Sound::getVolume() const {
	if (!exists()) return 0;
	return ma_sound_get_volume(&instance->source);
}

Engine::Sound& Engine::Sound::fade(float const from, float const to, float const time) {
	if (!exists()) return *this;
	ma_sound_set_fade_in_pcm_frames(&instance->source, from, to, instance->toPCMFrames(time));
	return *this;
}

Engine::Sound& Engine::Sound::setSpatial(bool const state) {
	if (!exists()) return *this;
	//ma_sound_set_spatialization_enabled(&instance->source, state ? MA_TRUE : MA_FALSE);
	return *this;
}

Instance<Engine::Sound> Engine::Sound::clone() const {
	if (!exists() || !instance->engine) return nullptr;
	return instance->engine->createSound(instance->data, instance->type, instance->group);
}

Instance<Engine::Group> Engine::Group::clone() const {
	if (!exists() || !instance->engine) return nullptr;
	return instance->engine->createGroup(instance->parent);
}

Engine::Group& Engine::Group::setVolume(float const volume) {
	if (!exists()) return *this;
	ma_sound_group_set_volume(&instance->group, volume);
	return *this;
}

float Engine::Group::getVolume() const {
	if (!exists()) return 0;
	return ma_sound_group_get_volume(&instance->group);
}

Engine& Engine::setVolume(float const volume) {
	if (!exists()) return *this;
	ma_engine_set_volume(&instance->engine, volume);
	return *this;
}

float Engine::getVolume() const {
	if (!exists()) return 0;
	return ma_engine_get_volume(&instance->engine);
}