#ifndef MAKAILIB_AUDIO_PLAYABLE_H
#define MAKAILIB_AUDIO_PLAYABLE_H

#include "../compat/ctl.hpp"

#include "../core/extern.hpp"
#include "../data/encdec.hpp"

/// @brief Audio facilities.
namespace Makai::Audio {
	/// @brief Audio source interface.
	class ISource {
	public:
		using Callback = const Signal<>;

		/// @brief Empty constructor.
		ISource();

		/// @brief Creates the source from a file.
		/// @param path Path to audio file.
		ISource(String const& path);

		/// @brief Destructor.
		virtual ~ISource();

		/// @brief Creates the source from a file.
		/// @param path Path to audio file.
		void create(String const& path);

		/// @brief Destroys the source.
		void destroy();

		/// @brief Called when playback is finished.
		virtual void onPlaybackEnd()	{}
		/// @brief Called when playback is started.
		virtual void onPlaybackStart()	{}

		/// @brief Returns whether the source exists.
		/// @return Whether source exists.
		bool exists();

		/// @brief
		///		Updates the audio subsystem.
		///		Not required to call every cycle, as the program already handles it.
		static void update();

	protected:
		/// @brief Called when the source is created. Must be implemented.
		/// @param data Audio source.
		virtual void onCreate(Extern::Resource const& data)	= 0;
		/// @brief Called when the source is destroyed.
		virtual void onDestroy()							= 0;
		/// @brief Called when the source is updated.
		virtual void onUpdate()								{}

	private:
		/// @brief Callback executed when updating.
		Callback yield = [&] {this->onUpdate();};

		/// @brief Audio file data.
		BinaryData<> data;

		/// @brief Whether the source was created.
		bool created = false;
	};

	/// @brief Audio callback.
	using AudioCallback = typename ISource::Callback;
}

#endif // MAKAILIB_AUDIO_PLAYABLE_H
