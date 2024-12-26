#ifndef MAKAILIB_AUDIO_UPDATE_HPP
#define MAKAILIB_AUDIO_UPDATE_HPP

#include "sound.hpp"
#include "music.hpp"

/// @brief Audio facilities.
namespace Makai::Audio {
	/// @brief Updates the audio & music subsystem.
	inline void updateAll() {
		Music::update();
		ISource::update();
	}
}

#endif // MAKAILIB_AUDIO_UPDATE_HPP
