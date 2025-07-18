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

struct Audio::Source::Content {
	BinaryData<>		file;
	owner<Mix_Chunk>	source	= nullptr;
	SourceType			type	= SourceType::ST_SOUND;
	uint				track	= -1;

	~Content() {
		if (!isOpen()) return;
		if (source) Mix_FreeChunk(source);
	}

	bool active() {
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

List<QueueInfo> queue;

SourceRef currentMusic = nullptr;

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
		Mix_FadeInChunk(nextAudioTrack(content), content.source, loops, fadeInTime);
	else Mix_PlayChunk(nextAudioTrack(content), content.source, loops);
}

static void playMusic(Source::Content& content, uint const fadeInTime, int const loops) {
	if (fadeInTime)
		Mix_FadeInChunk(nextMusicTrack(content), content.source, loops, fadeInTime);
	else Mix_PlayChunk(nextMusicTrack(content), content.source, loops);
}

static void playBasedOnType(Source::Content& content, uint const fadeInTime, int const loops) {
	switch (content.type) {
		case SourceType::ST_MUSIC: return playMusic(content, fadeInTime, loops);
		case SourceType::ST_SOUND: return playAudio(content, fadeInTime, loops);
	}
}

Source::Source(): APeriodicAudio() {
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
	if (!exists()) return;
	stop();
	queue.eraseIf([&] (QueueInfo const& info) {return info.content == data.raw();});
	if (currentMusic == data.raw()) currentMusic = nullptr;
	data.unbind();
	created = false;
};

bool Source::exists() const {
	return created && data.exists();
}

bool Source::playing() const {
	return exists() && data->active();
}

constexpr float const VOLUME_FACTOR = 128.0;

void Source::setVolume(float const volume) {
	if (!exists()) return;
	Mix_VolumeChunk(data->source, Math::round(Math::clamp<float>(volume, 0, 1) * VOLUME_FACTOR));
}

float Source::getVolume() const {
	if (!exists()) return 0;
	return Mix_VolumeChunk(data->source, -1) / VOLUME_FACTOR;
}

void Source::setMasterVolume(float const volume, SourceType const type) {
	switch (type) {
		// First few tracks are allocated for music, the rest is for sound
		case SourceType::ST_MUSIC: {
			auto const trackCount = getMusicTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				Mix_Volume(i, volume);
		} break;
		case SourceType::ST_SOUND: {
			auto const trackCount = getAudioTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				Mix_Volume(i + getMusicTrackCount(), volume);
		} break;
	}
}

float Source::getMasterVolume(SourceType const type) {
	switch (type) {
		// First few tracks are allocated for music, the rest is for sound
		case SourceType::ST_MUSIC: {
			if (getMusicTrackCount() > 0)
				return Mix_Volume(0, -1) / VOLUME_FACTOR;
		} break;
		case SourceType::ST_SOUND: {
			if (getAudioTrackCount() > 0)
				return Mix_Volume(getMusicTrackCount() + 1, -1) / VOLUME_FACTOR;
			return 0;
		} break;
	}
}

static void updateMusicQueue() {
	if (queue.empty()) return;
	if (currentMusic && currentMusic->active()) return;
	auto const song = queue.front();
	currentMusic = song.content;
	if (song.content)
		playMusic(*song.content, song.fadeInTime, song.loops);
	queue.erase(0);
}

void Source::process() {
	APeriodicAudio::process();
	updateMusicQueue();
}

void Source::masterStop(uint const fadeOutTime, SourceType const type) {
	switch (type) {
		// First few tracks are allocated for music, the rest is for sound
		case SourceType::ST_MUSIC: {
			auto const trackCount = getMusicTrackCount();
			for (uint i = 0; i < trackCount; ++i)
				if (fadeOutTime) Mix_HaltChannel(i, fadeOutTime);
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

void Source::onUpdate() {
	playedThisFrame = false;
	if (cooldown > 0) --cooldown;
}

void Source::play(
	int const	loops,
	uint const	fadeInTime,
	bool const	force
) {
	if (!exists()) return;
	if (!force && data->active()) return;
	if (cooldown) return;
	if (data->type == SourceType::ST_MUSIC) {
		stopMusic();
		currentMusic = data.raw();
	}
	playBasedOnType(content, fadeInTime, loops);
}

void Source::playOnceThisFrame(
	int const	loops,
	uint const	fadeInTime,
	bool const	force
) {
	if (!exists()) return;
	if (!force && data->active()) return;
	playOnceAndWait(content, fadeInTime, loops, 1);
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
	play(content, fadeInTime, loops);
	cooldown = cycles;
}

void Source::crossFadeInto(uint const crossfadeTime) {
	if (!exists()) return;
	if (!force && data->active()) return;
	if (cooldown) return;
	stopMusic(crossFadeTime);
	playBasedOnType(content, fadeInTime, 0);
}

void Source::crossFadeInto(uint const crossfadeTime, int const loops) {
	if (!exists()) return;
	if (!force && data->active()) return;
	if (cooldown) return;
	stopMusic(crossFadeTime);
	playBasedOnType(content, fadeInTime, loops);
}