#ifndef MAKAILIB_AUDIO_CORE_H
#define MAKAILIB_AUDIO_CORE_H

#include "../compat/ctl.hpp"

/// @brief Audio facilities. 
namespace Makai::Audio {
	template <class T> struct Component {
		struct Resource;
	protected:
		Instance<Resource> instance;
	};
}

#endif // MAKAILIB_AUDIO_CORE_H
