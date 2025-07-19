#ifndef MAKAILIB_AUDIO_UPDATE_HPP
#define MAKAILIB_AUDIO_UPDATE_HPP

#include "source.hpp"

/// @brief Audio facilities.
namespace Makai::Audio {
	/// @brief Updates the audio & music subsystem.
	inline void updateAll() {
		Source::process();
	}
}

#endif // MAKAILIB_AUDIO_UPDATE_HPP
