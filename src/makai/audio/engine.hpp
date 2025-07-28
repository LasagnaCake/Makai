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

		Instance<Group> createGroup(Handle<Group> const& parent);

		Instance<Sound> createSound(
			BinaryData<> const&		data,
			LoadMode const			mode	= LoadMode::LM_PRELOAD,
			Handle<Group> const&	group	= {nullptr}
		);

		using Component<Engine>::exists;
	
	private:
		using Component<Engine>::instance;
	};
	
	struct Engine::Group: Component<Engine::Group> {
		using typename Component<Group>::Resource;

		using Component<Group>::exists;

		~Group();

	private:
		Group();
		using Component<Engine::Group>::instance;
		friend struct Engine;
	};
	
	struct Engine::Sound: Component<Engine::Sound>, IPlayable, ILoud {
		using typename Component<Sound>::Resource;

		using Component<Sound>::exists;

		void setLooping(bool const state = false);
		bool looping();

		Sound& start() override;
		Sound& start(bool const loop, usize const fadeIn = 0);

		Sound& play() override;
		Sound& pause() override;
		Sound& stop() override;
		Sound& stop(usize const fadeOut);

		void	setVolume(float const volume)	override;
		float	getVolume() const				override;

		virtual ~Sound();

	private:
		Sound();
		using Component<Engine::Sound>::instance;
		friend struct Engine;
	};
}

#endif // MAKAILIB_AUDIO_ENGINE_H
