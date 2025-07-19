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

using SourceType = Source::SourceType;

constexpr float const SDL_VOLUME_FACTOR = 128.0;

inline static int8 toSDLVolume(float const volume) {
	return CTL::Math::round(CTL::Math::clamp<float>(volume, 0, 1) * SDL_VOLUME_FACTOR);
}

struct Audio::Source::Content {
	BinaryData<>		file;
	owner<Mix_Chunk>	source	= nullptr;
	SourceType			type	= SourceType::ST_SOUND;
	int					track	= -1;
	float				volume = 1, spaceVolume = 1;

	float trueVolume() {
		return volume * spaceVolume;
	}

	uint8 sdlVolume() {
		return toSDLVolume(trueVolume());
	}

	~Content() {
		if (!isOpen()) return;
		if (source) Mix_FreeChunk(source);
	}

	bool active() {
		if (!isOpen())		return false;
		if (!source)		return false;
		if (track == -1)	return false;
		return (
			Mix_Playing(track)
		&&	Mix_GetChunk(track) == source
		);
	}
};

using SourceRef = ref<Source::Content>;

struct QueueInfo {
	SourceRef	content;
	uint		fadeInTime;
	int			loops;
};

usize currentMusicTrack = 0;
usize currentAudioTrack = 0;

static uint nextAudioTrack(Source::Content& content) {
	if (content.track > -1) return content.track;
	if (auto const trackCount = getAudioTrackCount()) {
		auto const otherTracks = getMusicTrackCount();
		if ((Mix_Playing(-1) - otherTracks) < trackCount) {
			while (Mix_Playing(currentAudioTrack + otherTracks))
				currentAudioTrack = (currentAudioTrack+1) % trackCount;
		}
	}
	return content.track = currentAudioTrack + getMusicTrackCount();
}

static uint nextMusicTrack(Source::Content& content) {
	if (content.track > -1) return content.track;
	if (auto const trackCount = getMusicTrackCount()) {
		auto const otherTracks = getAudioTrackCount();
		if ((Mix_Playing(-1) - otherTracks) < trackCount) {
			while (Mix_Playing(currentMusicTrack))
				currentMusicTrack = (currentMusicTrack+1) % trackCount;
		}
	}
	return content.track = currentMusicTrack;
}

static void playAudio(Source::Content& content, uint const fadeInTime, int const loops) {
	if (fadeInTime)
		Mix_FadeInChannel(nextAudioTrack(content), content.source, loops, fadeInTime);
	else Mix_PlayChannel(nextAudioTrack(content), content.source, loops);
}

static void playMusic(Source::Content& content, uint const fadeInTime, int const loops) {
	if (fadeInTime)
		Mix_FadeInChannel(nextMusicTrack(content), content.source, loops, fadeInTime);
	else Mix_PlayChannel(nextMusicTrack(content), content.source, loops);
}

static void playBasedOnType(Source::Content& content, uint const fadeInTime, int const loops) {
	switch (content.type) {
		case SourceType::ST_MUSIC: return playMusic(content, fadeInTime, loops);
		case SourceType::ST_SOUND: return playAudio(content, fadeInTime, loops);
	}
}

Source::Source(): APeriodicSource() {
}

Source::Source(String const& path, SourceType const type): Source() {
	create(path, type);
}

Source::~Source() {
	DEBUGLN("Deleting audio source object...");
	destroy();
	DEBUGLN("Object deleted!");
}

void Source::create(String const& path, SourceType const type) {
	if (exists()) return;
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
			toString("Could not load audio file [", path, "]!"),
			String(Mix_GetError()),
			CTL_CPP_PRETTY_SOURCE
		);
	created = true;
};

void Source::setType(SourceType const type) {
	if (exists()) {
		stop();
		data->type = type;
	}
};

void Source::destroy() {
	if (!(isOpen() && exists())) return;
	stop();
	created = false;
};

bool Source::exists() const {
	return created && data.exists();
}

bool Source::playing() const {
	return exists() && data->active();
}

bool Source::isMusic() const {
	return exists() && data->type == SourceType::ST_MUSIC;
}

void Source::setMasterVolume(float const volume, SourceType const type) {
	switch (type) {
		// First few tracks are allocated for music, the rest is for sound
		case SourceType::ST_MUSIC: {
			auto const trackCount = getMusicTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				Mix_Volume(i, toSDLVolume(volume));
		} break;
		case SourceType::ST_SOUND: {
			auto const trackCount = getAudioTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				Mix_Volume(i + getMusicTrackCount(), toSDLVolume(volume));
		} break;
	}
}

float Source::getMasterVolume(SourceType const type) {
	switch (type) {
		// First few tracks are allocated for music, the rest is for sound
		case SourceType::ST_MUSIC: {
			if (getMusicTrackCount() > 0)
				return Mix_Volume(0, -1) / SDL_VOLUME_FACTOR;
		} break;
		case SourceType::ST_SOUND: {
			if (getAudioTrackCount() > 0)
				return Mix_Volume(getMusicTrackCount() + 1, -1) / SDL_VOLUME_FACTOR;
			return 0;
		} break;
	}
	return 0;
}

void Source::process() {
	APeriodicSource::process();
}

void Source::masterStop(uint const fadeOutTime, SourceType const type) {
	switch (type) {
		// First few tracks are allocated for music, the rest is for sound
		case SourceType::ST_MUSIC: {
			auto const trackCount = getMusicTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				if (fadeOutTime) Mix_FadeOutChannel(i, fadeOutTime);
				else Mix_HaltChannel(i);
		} break;
		case SourceType::ST_SOUND: {
			auto const trackCount = getAudioTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				if (fadeOutTime) Mix_FadeOutChannel(i + getMusicTrackCount(), fadeOutTime);
				else Mix_HaltChannel(i + getMusicTrackCount());
		} break;
	}
}

void Source::masterPause(SourceType const type) {
	switch (type) {
		// First few tracks are allocated for music, the rest is for sound
		case SourceType::ST_MUSIC: {
			auto const trackCount = getMusicTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				Mix_Pause(i);
		} break;
		case SourceType::ST_SOUND: {
			auto const trackCount = getAudioTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				Mix_Pause(i + getMusicTrackCount());
		} break;
	}
}

void Source::masterUnpause(SourceType const type) {
	switch (type) {
		// First few tracks are allocated for music, the rest is for sound
		case SourceType::ST_MUSIC: {
			auto const trackCount = getMusicTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				Mix_Resume(i);
		} break;
		case SourceType::ST_SOUND: {
			auto const trackCount = getAudioTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				Mix_Resume(i + getMusicTrackCount());
		} break;
	}
}

void Source::stop(uint const fadeOutTime) {
	if (!(exists() && data->active())) return;
	if (fadeOutTime) Mix_FadeOutChannel(data->track, fadeOutTime);
	else Mix_HaltChannel(data->track);
}

void Source::pause() {
	if (!exists()) return;
	if (data->active()) Mix_Pause(data->track);
}

void Source::unpause() {
	if (!exists()) return;
	if (data->active()) Mix_Resume(data->track);
}

inline static float volumeByDistance(float const distance) {
	return CTL::Math::clamp<float>(1 - CTL::Math::sqrt<float>(distance), 0, 1);
}

void Source::updateVolume() {
	constexpr float const SDL_PAN_FACTOR = 255;
	data->volume = volume;
	if (!spatial || (world.size.x == 0 && world.size.y == 0)) {
		data->spaceVolume = 1;
		Mix_VolumeChunk(data->source, data->sdlVolume());
	} else if (world.size.x == 0) {
		float const space = ((listener.position.y - position.y) / world.size.y);
		data->spaceVolume = volumeByDistance(space);
		Mix_VolumeChunk(data->source, data->sdlVolume());
	} else if (world.size.y == 0) {
		float const pan		= (listener.position.x - position.x) / world.size.x;
		data->spaceVolume	= volumeByDistance(Math::abs(pan));
		float const left	= Math::clamp<float>(-pan + 0.5, 0, 1) * SDL_PAN_FACTOR;
		float const right	= Math::clamp<float>(pan + 0.5, 0, 1) * SDL_PAN_FACTOR;
		Mix_SetPanning(data->track, Math::round(left), Math::round(right));
		Mix_VolumeChunk(data->source, data->sdlVolume());
	} else {
		Vector2 const space = ((listener.position - position) / world.size);
		data->spaceVolume	= volumeByDistance(space.length());
		if (data->spaceVolume == 1) {
			Mix_SetPanning(data->track, 255, 255);
			Mix_VolumeChunk(data->source, data->sdlVolume());
			return;
		}
		float const pan		= Math::cos(space.angle());
		float const left	= Math::clamp<float>(1 - pan, 0, 1) * SDL_PAN_FACTOR;
		float const right	= Math::clamp<float>(1 + pan, 0, 1) * SDL_PAN_FACTOR;
		Mix_SetPanning(data->track, Math::round(left), Math::round(right));
		Mix_VolumeChunk(data->source, data->sdlVolume());
	} 
}

void Source::onUpdate() {
	if (cooldown > 0) --cooldown;
	if (wasPlaying && !playing()) onPlaybackEnd();
	if (!wasPlaying && playing()) onPlaybackStart();
	wasPlaying = playing();
	if (!wasPlaying) return;
	if (spatial && world.size.x != 0 && world.size.y != 0)
		updateVolume();
}

void Source::play(
	int const	loops,
	uint const	fadeInTime,
	bool const	force
) {
	if (!exists()) return;
	if (!force && data->active()) return;
	if (cooldown) return;
	playBasedOnType(*data, fadeInTime, loops);
	updateVolume();
}

void Source::playOnceThisFrame(
	int const	loops,
	uint const	fadeInTime,
	bool const	force
) {
	if (!exists()) return;
	if (!force && data->active()) return;
	playOnceAndWait(loops, fadeInTime, force, 1);
}

void Source::playOnceAndWait(
	int const	loops,
	uint const	fadeInTime,
	bool const	force,
	usize const	cycles
) {
	if (!exists()) return;
	if (!force && data->active()) return;
	if (cooldown) return;
	play(loops, fadeInTime, force);
	cooldown = cycles;
}

void Source::crossFadeInto(uint const crossFadeTime, int const loops) {
	if (!exists()) return;
	if (data->active()) return;
	if (cooldown) return;
	if (data->type != SourceType::ST_MUSIC) return;
	stopMusic(crossFadeTime);
	playBasedOnType(*data, crossFadeTime, loops);
	updateVolume();
}

void Source::setSpatial(bool const spatial) {
	this->spatial = spatial;
	updateVolume();
}