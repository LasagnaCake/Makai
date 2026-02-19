#include "engine.hpp"
#include "../file/file.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#include <windows.h>
#define SDL_MAIN_HANDLED
#endif
#include <miniaudio.h>

using namespace Makai::Audio;
using namespace Makai;

template<>
struct Component<Engine>::Resource {
	ma_engine engine;

	List<Instance<Engine::Sound::Resource>> sounds;
	List<Instance<Engine::Group::Resource>>	groups;

	Instance<Engine::Sound> createSound(
		BinaryData<> const&						data,
		Engine::SoundType const					mode,
		Handle<Engine::Group::Resource> const&	group
	);
	Instance<Engine::Group> createGroup(Handle<Engine::Group::Resource> const& parent);

	Resource();

	~Resource();
};

template<>
struct Component<Engine::Sound>::Resource {
	ma_sound			source;
	ma_decoder			decoder;
	ma_decoder_config	config;

	BinaryData<>					data;
	Handle<Engine::Resource>		engine;
	Handle<Engine::Group::Resource>	group;

	Engine::SoundType type;

	usize cooldown = 0;

	bool paused = false;

	void update() {
		if (cooldown) --cooldown;
	}

	bool locked() const {return ((ma_sound_is_playing(&source) == MA_TRUE) && cooldown);}

	usize toPCMFrames(float const time) const {
		return getSampleRate() * time;
	}

	float toSeconds(usize const time) const {
		return time / static_cast<float>(getSampleRate());
	}

	ma_uint32 getSampleRate() const {
		return ma_engine_get_sample_rate(ma_sound_get_engine(&source));
	}

	~Resource();
};

template<>
struct Component<Engine::Group>::Resource {
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
	Instance<Engine::Resource>::detach(this);
}

Engine::Sound::Resource::~Resource() {
	if (!engine) return;
	ma_sound_uninit(&source);
	ma_decoder_uninit(&decoder);
}

Engine::Group::Resource::~Resource() {
	if (!engine) return;
	ma_sound_group_uninit(&group);
}

Engine::Engine()			{							}
Engine::~Engine()			{close();					}
Engine::Sound::Sound()		{instance = new Resource();	}
Engine::Sound::~Sound()		{							}
Engine::Group::Group()		{instance = new Resource();	}
Engine::Group::~Group()		{							}

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
	return 0;
}

Instance<Engine::Group> Engine::Resource::createGroup(Handle<Engine::Group::Resource> const& parent) {
	Instance<Engine::Group> group = new Engine::Group();
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
	return instance->createGroup(((parent) ? parent->instance.asWeak() : nullptr));
}

Instance<Engine::Sound> Engine::Resource::createSound(
	BinaryData<> const& data,
	Engine::SoundType const type,
	Handle<Engine::Group::Resource> const& group
) {
	if (data.empty()) return nullptr;
	Instance<Engine::Sound> sound = new Engine::Sound();
	sound->instance->config = ma_decoder_config_init(
		ma_format_f32,
		ma_engine_get_channels(&engine),
		ma_engine_get_sample_rate(&engine)
	);
	sound->instance->config.encodingFormat = ma_encoding_format_unknown;
	sound->instance->engine = this;
	sound->instance->group = group;
	sound->instance->type = type;
	sound->instance->data = data;
	DEBUGLN("Creating decoder...");
	ma_result result;
	if (
		(result = ma_decoder_init_memory(
			sound->instance->data.data(),
			sound->instance->data.size(),
			&sound->instance->config,
			&sound->instance->decoder
		)) != MA_SUCCESS
	) {
		DEBUGLN("ERROR: ", ma_result_description(result));
		return nullptr;
	}
	DEBUGLN("Creating sound instance...");
	if (
		(result = ma_sound_init_from_data_source(
			&engine,
			&sound->instance->decoder,
			modeFlags(type) | MA_SOUND_FLAG_NO_SPATIALIZATION,
			group ? &group->group : NULL,
			&sound->instance->source
		)) != MA_SUCCESS
	) {
		DEBUGLN("ERROR: ", ma_result_description(result));
		return nullptr;
	}
	DEBUGLN("Done!");
	sounds.pushBack(sound->instance);
	return sound;
}

Instance<Engine::Sound> Engine::createSound(BinaryData<> const& data, SoundType const type, Handle<Group> const& group) {
	if (!exists()) return nullptr;
	return instance->createSound(data, type, ((group) ? group->instance.asWeak() : nullptr));
}

Instance<Engine::Sound> Engine::createSound(String const& file, SoundType const type, Handle<Group> const& group) {
	return createSound(File::getBinary(file), type, group);
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
	if (instance->locked()) return * this;
	if (playing() || paused()) {
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
	instance->cooldown = 0;
	return *this;
}

Engine::Sound& Engine::Sound::setLooping(bool const loop) {
	if (!exists()) return *this;
	ma_sound_set_looping(&instance->source, loop ? MA_TRUE : MA_FALSE);
	return *this;
}

Engine::Sound& Engine::Sound::setLoopPoints(float const begin, float end) {
	if (!exists() || begin < 0) return *this;
	ma_uint64 start = 0, stop = 0;
	if (ma_sound_get_length_in_pcm_frames(&instance->source, &stop) != MA_SUCCESS)
		return *this;
	--stop;
	if (end >= 0)
		stop = Math::min<uint64>(instance->toPCMFrames(end), stop);
	start = instance->toPCMFrames(begin);
	if (stop <= start) return *this;
	DEBUGLN("<loop>");
	DEBUGLN("    BEGIN: ", begin, " (", start, ")");
	DEBUGLN("    END:   ", end, " (", stop, ")");
	DEBUGLN("</loop>");
	ma_data_source_set_loop_point_in_pcm_frames(&instance->decoder, start, stop);
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
