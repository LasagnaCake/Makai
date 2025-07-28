#ifndef MAKAILIB_AUDIO_CORE_H
#define MAKAILIB_AUDIO_CORE_H

#include "../compat/ctl.hpp"

/// @brief Audio facilities. 
namespace Makai::Audio {
	struct ILoud {
		constexpr ~ILoud() {}

		virtual void	setVolume(float const volume)	= 0;
		virtual float	getVolume() const				= 0;
	};

	template <class T> struct Component {
		struct Resource;
		
		/// @brief Returns whether the component exists.
		/// @return Whether component exists.
		bool exists() const {return instance;}

	protected:
		Instance<Resource> instance;
	};
}

#endif // MAKAILIB_AUDIO_CORE_H
