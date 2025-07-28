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

		/// @brief Destructor.
		virtual ~Group();
	
		/// @brief Copy assignment operator (defaulted).
		Group& operator=(Group const& other)	= delete;
		/// @brief Move assignment operator (defaulted).
		Group& operator=(Group&& other)			= delete;

		/// @brief Creates a copy of the sound group.
		/// @return New copy of sound group.
		Instance<Group> clone() const override final;
		/// @brief Creates a copy of the sound group.
		/// @return New copy of sound group.
		Instance<Group> clone() override final {return constant(*this).clone();};

		/// @brief Sets the sound group's volume.
		/// @param volume Volume to set to.
		Group&	setVolume(float const volume)	override final;
		/// @brief Returns the sound group's volume.
		/// @return Group volume.
		float	getVolume() const				override final;

	private:
		Group();
		using Component<Engine::Group>::instance;
		friend struct Engine;
		friend struct Engine::Resource;
	};
	
	struct Engine::Sound: Component<Engine::Sound>, ILoud, IClonable<Instance<Engine::Sound>> {
		using typename Component<Sound>::Resource;

		using Component<Sound>::exists;

		/// @brief Destructor.
		virtual ~Sound();

		/// @brief Copy assignment operator (defaulted).
		Sound& operator=(Sound const& other)	= delete;
		/// @brief Move assignment operator (defaulted).
		Sound& operator=(Sound&& other)			= delete;

		/// @brief Enables/disables looping.
		/// @param Whether to enable (`true`) or disable (`false`) looping.
		/// @return Reference to self.
		Sound& setLooping(bool const state = true);
		/// @brief Returns whether the sound is set to loop.
		/// @return Whether sound is set to loop.
		bool looping();

		/// @brief Returns whether sound is currently playing.
		/// @return Whether sound is playing.
		bool playing();

		/// @brief Plays the sound.
		/// @param force Whether to force sound to play from the start, if already playing. Does not ignore cooldown. By default, it is `false`.
		/// @param loop Whether sound should loop indefinitely. By default, it is `false`.
		/// @brief fadeIn Fade-in time, in seconds. By default, it is zero.
		/// @brief cooldown Cooldown before sound can be played again, in cycles. By default, it is zero.
		/// @return Reference to self.
		Sound& play(
			bool const	force		= false,
			bool const	loop		= false,
			float const	fadeIn		= 0,
			usize const	cooldown	= 0
		);
		/// @brief Stops the sound.
		/// @brief fadeOut Fade-out time, in seconds. By default, it is zero.
		/// @return Reference to self.
		Sound& stop(float const fadeOut = 0);

		Sound& unpause();
		Sound& pause();

		Sound&	fade(float const from, float const to, float const time);
		Sound&	fadeTo(float const volume, float const time)				{fade(-1, volume, time); return *this;	}
		Sound&	fadeIn(float const time)									{fadeTo(1, time); return *this;			}
		Sound&	fadeOut(float const time)									{fadeTo(0, time); return *this;			}

		Sound&	setPlaybackTime(float const time);
		float	getPlaybackTime() const;

		/// @brief Sets the sound's volume.
		/// @param volume Volume to set to.
		Sound&	setVolume(float const volume)	override final;
		/// @brief Returns the sound's volume.
		/// @return Sound volume.
		float	getVolume() const				override final;

		Sound& setSpatial(bool const state);

		/// @brief Creates a copy of the sound.
		/// @return New copy of sound group.
		Instance<Sound> clone() const override final;
		/// @brief Creates a copy of the sound.
		/// @return New copy of sound group.
		Instance<Sound> clone() override final {return constant(*this).clone();};

	private:
		Sound();
		using Component<Engine::Sound>::instance;
		friend struct Engine;
		friend struct Engine::Resource;
	};
}

#endif // MAKAILIB_AUDIO_ENGINE_H
