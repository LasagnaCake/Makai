#ifndef MAKAILIB_AUDIO_CORE_H
#define MAKAILIB_AUDIO_CORE_H

#include "../compat/ctl.hpp"

/// @brief Audio facilities. 
namespace Makai::Audio {
	/// @brief Interface for an object with volume controls.
	struct ILoud {
		/// @brief Destructor.
		constexpr ~ILoud() {}

		/// @brief Sets the current volume. MUST be implemented.
		virtual ILoud&	setVolume(float const volume)	= 0;
		/// @brief Returns the current volume. MUST be implemented.
		virtual float	getVolume() const				= 0;
	};

	/// @brief Audio component.
	/// @tparam Component type.
	template <class T>
	struct Component {
		/// @brief Audio component resource. Implementation dependent on `T`.
		/// @note Generally, it should be an opaque type. Hence why it has no definition.
		struct Resource;

		/// @brief Returns whether the component exists.
		/// @return Whether component exists.
		bool exists() const {return instance;}

	protected:
		/// @brief Resource instance.
		Instance<Resource> instance;
	};
}

#endif // MAKAILIB_AUDIO_CORE_H
