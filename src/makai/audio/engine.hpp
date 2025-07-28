#ifndef MAKAILIB_AUDIO_ENGINE_H
#define MAKAILIB_AUDIO_ENGINE_H

#include "../compat/ctl.hpp"
#include "core.hpp"

/// @brief Audio facilities. 
namespace Makai::Audio {
	/// @brief Audio engine.
	struct Engine;

	using APeriodicAudioEvent = APeriodic<Engine>;
	
	/// @brief Audio engine.
	struct Engine: private Component<Engine>, public APeriodicAudioEvent, ILoud {
		using typename Component<Engine>::Resource;

		/// @brief Engine sound.
		struct Sound;
		/// @brief Engine sound roup.
		struct Group;

		/// @brief Engine sound type.
		enum class SoundType {
			/// @brief Streamed sound, decoded on-the-fly. Better for music.
			EST_STREAMED,
			/// @brief Pre-decode the sound, in the current thread. Better for SFX.
			EST_PRELOADED,
			/// @brief Pre-decode the sound, in the background.
			EST_PRELOADED_ASYNC
		};

		/// @brief Default constructor.
		Engine();

		/// @brief Destructor.
		~Engine();

		/// @brief Copy assignment operator (defaulted).
		Engine& operator=(Engine const& other)	= default;
		/// @brief Move assignment operator (defaulted).
		Engine& operator=(Engine&& other)		= default;

		/// @brief Stops all audio playback currently happening.
		void stopAllSounds();

		/// @brief Opens the audio engine.
		void open();

		/// @brief Closes the audio engine.
		void close();

		/// @brief Updates the audio engine.
		void onUpdate() override;

		/// @brief Sets the engine's master volume.
		/// @param volume Volume to set to.
		Engine&	setVolume(float const volume)	override final;
		/// @brief Returns the engine's master volume.
		/// @return Master volume.
		float	getVolume() const				override final;

		/// @brief Creates a sound group in the engine.
		/// @param parent Group parent. By default, it is `nullptr` (i.e. none).
		Instance<Group> createGroup(Handle<Group> const& parent = nullptr);

		/// @brief Creates a sound in the engine.
		/// @param parent Group parent. By default, it is `nullptr`.
		Instance<Sound> createSound(
			BinaryData<> const&		data,
			SoundType const			mode	= SoundType::EST_PRELOADED,
			Handle<Group> const&	group	= {nullptr}
		);

		using Component<Engine>::exists;
	
	private:
		using Component<Engine>::instance;
	};

	struct Engine::Group: Component<Engine::Group>, IClonable<Instance<Engine::Group>>, ILoud {
		using typename Component<Group>::Resource;

		using Component<Group>::exists;

		virtual ~Group();

		Group& operator=(Group const& other)	= delete;
		Group& operator=(Group&& other)			= delete;

		Instance<Group> clone() const override final;
		Instance<Group> clone() override final {return constant(*this).clone();};

		Group&	setVolume(float const volume)	override final;
		float	getVolume() const				override final;

	private:
		Group();
		using Component<Engine::Group>::instance;
		friend struct Engine;
		friend struct Engine::Resource;
	};
	
	struct Engine::Sound: Component<Engine::Sound>, IPlayable, ILoud, IClonable<Instance<Engine::Sound>> {
		using typename Component<Sound>::Resource;

		using Component<Sound>::exists;

		virtual ~Sound();

		Sound& operator=(Sound const& other)	= delete;
		Sound& operator=(Sound&& other)			= delete;

		Sound& setLooping(bool const state = false);
		bool looping();
		bool playing();

		Sound& start() override final;
		Sound& start(bool const force, bool const loop, float const fadeIn = 0, usize const cooldown = 1);

		Sound& play() override final;
		Sound& pause() override final;
		Sound& stop() override final;
		Sound& stop(float const fadeOut);

		Sound&	fade(float const from, float const to, float const time);
		Sound&	fadeTo(float const volume, float const time)				{fade(-1, volume, time); return *this;	}
		Sound&	fadeIn(float const time)									{fadeTo(1, time); return *this;			}
		Sound&	fadeOut(float const time)									{fadeTo(0, time); return *this;			}

		Sound&	setPlaybackTime(float const time);
		float	getPlaybackTime() const;

		Sound&	setVolume(float const volume)	override final;
		float	getVolume() const				override final;

		Sound& setSpatial(bool const state);

		Instance<Sound> clone() const override final;
		Instance<Sound> clone() override final {return constant(*this).clone();};

	private:
		Sound();
		using Component<Engine::Sound>::instance;
		friend struct Engine;
		friend struct Engine::Resource;
	};
}

#endif // MAKAILIB_AUDIO_ENGINE_H
