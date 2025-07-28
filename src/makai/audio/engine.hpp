#ifndef MAKAILIB_AUDIO_ENGINE_H
#define MAKAILIB_AUDIO_ENGINE_H

#include "../compat/ctl.hpp"
#include "core.hpp"

/// @brief Audio facilities. 
namespace Makai::Audio {
	struct Engine;

	struct Engine: private Component<Engine> {
		using typename Component<Engine>::Resource;

		struct Sound;
		struct Group;

		enum class LoadMode {
			LM_STREAM,
			LM_PRELOAD,
			LM_PRELOAD_ASYNC
		};

		Engine();

		~Engine();

		/// @brief Stops all audio playback currently happening.
		void stopAllSounds();

		/// @brief Opens the audio engine.
		void open();

		/// @brief Closes the audio engine.
		void close();

		Instance<Group> createGroup(Handle<Engine::Group> const& parent);

		Instance<Sound> createSound(
			BinaryData<> const&		data,
			LoadMode const			mode	= LoadMode::LM_PRELOAD,
			Handle<Group> const&	group	= {nullptr}
		);

		/// @brief Returns whether the audio engine is open.
		/// @return Whether engine is open.
		bool exists() const;
	
	private:
		using Component<Engine>::instance;
	};
	
	struct Engine::Group: Component<Engine::Group> {
		using typename Component<Engine::Group>::Resource;

		~Group();

	private:
		Group();
		using Component<Engine::Group>::instance;
		friend struct Engine;
	};
	
	struct Engine::Sound: Component<Engine::Sound> {
		using typename Component<Engine::Sound>::Resource;

		~Sound();

	private:
		Sound();
		using Component<Engine::Sound>::instance;
		friend struct Engine;
	};
}

#endif // MAKAILIB_AUDIO_ENGINE_H
