#ifndef MAKAILIB_AUDIO_CORE_H
#define MAKAILIB_AUDIO_CORE_H

#include "../compat/ctl.hpp"

/// @brief Audio facilities. 
namespace Makai::Audio {
	/// @brief Stops all audio playback currently happening.
	void stopAll();

	/// @brief Audio codec format.
	enum class Format {
		AF_NONE,
		AF_MP3,
		AF_OGG,
		AF_MIDI,
		AF_FLAC,
		AF_MOD,
		AF_OPUS,
	//	AF_WAVPACK,
		AF_MAX_FORMATS
	};

	/// @brief Audio format list.
	using Formats = List<Format>;

	/// @brief Opens the audio subsystem.
	/// @param formats Audio codecs to enable.
	/// @param channels Number of output channels. 1 → mono, 2 → stereo, and so on.
	/// @param audioTracks Number of audio (non-music) tracks.
	void open(Formats const& formats, uint const channels = 2, uint const audioTracks = 16);

	/// @brief Restarts the audio subsystem.
	/// @param formats Audio codecs to enable.
	/// @param channels Number of output channels. 1 → mono, 2 → stereo, and so on.
	/// @param audioTracks Number of audio (non-music) tracks.
	void restart(Formats const& formats, uint const channels = 2, uint const audioTracks = 16);

	/// @brief Closes the audio subsystem.
	void close();

	/// @brief Returns whether the audio subsystem is open.
	/// @return Whether audio subsystem is open.
	bool isOpen();
}

#endif // MAKAILIB_AUDIO_CORE_H
