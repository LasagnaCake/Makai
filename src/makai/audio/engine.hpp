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
	struct Engine: private Component<Engine>, APeriodicAudioEvent, ILoud {
		using typename Component<Engine>::Resource;

		/// @brief Engine sound.
		struct Sound;
		/// @brief Engine sound group.
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

		/// @brief Constructs and opens the engine.
		Engine(bool const): Engine() {open();};

		/// @brief Destructor.
		~Engine();

		/// @brief Copy assignment operator (deleted).
		Engine& operator=(Engine const& other)	= delete;
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
		/// @param data Sound file data.
		/// @param type Sound type. By default, it is `EST_PRELOADED`.
		/// @param parent Group parent. By default, it is `nullptr`.
		/// @return Sound instance, or `nullptr` on failure.
		Instance<Sound> createSound(
			BinaryData<> const&		data,
			SoundType const			type	= SoundType::EST_PRELOADED,
			Handle<Group> const&	group	= {nullptr}
		);

		/// @brief Creates a sound in the engine.
		/// @param file Sound file path.
		/// @param type Sound type. By default, it is `EST_PRELOADED`.
		/// @param parent Group parent. By default, it is `nullptr`.
		/// @return Sound instance, or `nullptr` on failure.
		Instance<Sound> createSound(
			String const&			file,
			SoundType const			type	= SoundType::EST_PRELOADED,
			Handle<Group> const&	group	= {nullptr}
		);

		using Component<Engine>::exists;
	
	private:
		using Component<Engine>::instance;
	};

	struct Engine::Group: private Component<Engine::Group>, IClonable<Instance<Engine::Group>>, ILoud {
		using typename Component<Group>::Resource;

		using Component<Group>::exists;

		/// @brief Destructor.
		virtual ~Group();
	
		/// @brief Copy assignment operator (deleted).
		Group& operator=(Group const& other)	= delete;
		/// @brief Move assignment operator (deleted).
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
	
	struct Engine::Sound: private Component<Engine::Sound>, ILoud, IClonable<Instance<Engine::Sound>> {
		using typename Component<Sound>::Resource;

		using Component<Sound>::exists;

		/// @brief Destructor.
		virtual ~Sound();

		/// @brief Copy assignment operator (deleted).
		Sound& operator=(Sound const& other)	= delete;
		/// @brief Move assignment operator (deleted).
		Sound& operator=(Sound&& other)			= delete;

		/// @brief Enables/disables looping.
		/// @param Whether to enable (`true`) or disable (`false`) looping.
		/// @return Reference to self.
		Sound& setLooping(bool const state = true);
		
		/// @brief Returns whether the sound is set to loop.
		/// @return Whether sound is set to loop.
		bool looping() const;

		/// @brief Returns whether sound is currently playing.
		/// @return Whether sound is playing.
		bool playing() const;

		/// @brief Returns whether sound is currently paused.
		/// @return Whether sound is paused.
		bool paused() const;

		/// @brief Returns whether sound is currently fully stopped.
		/// @return Whether sound is fully stopped.
		bool stopped() const {return !(playing() || paused());}

		/// @brief Plays the sound from the beginning.
		/// @param force Whether to force sound to play from the start, if already playing. Does not ignore cooldown. By default, it is `false`.
		/// @param loop Whether sound should loop indefinitely. By default, it is `false`.
		/// @brief fadeIn Fade-in duration, in seconds. By default, it is zero.
		/// @brief cooldown Cooldown before sound can be played again, in cycles. By default, it is zero.
		/// @return Reference to self.
		Sound& play(
			bool const	force		= false,
			bool const	loop		= false,
			float const	fadeIn		= 0,
			usize const	cooldown	= 0
		);
		/// @brief Stops the sound.
		/// @brief fadeOut Fade-out duration, in seconds. By default, it is zero.
		/// @return Reference to self.
		/// @note If audio must keep playing after fading out, use `fadeOut` instead.
		Sound& stop(float const fadeOut = 0);

		/// @brief Unpauses the sound.
		/// @return Reference to self.
		Sound& unpause();
		/// @brief Pauses the sound.
		/// @return Reference to self.
		Sound& pause();

		/// @brief Fades (but does not stop) the audio.
		/// @param from Volume to fade from. Use `-1` for current volume.
		/// @param from Volume to fade to. Use `-1` for current volume.
		/// @param time Fade duration, in seconds.
		/// @return Reference to self.
		Sound&	fade(float const from, float const to, float const time);
		/// @brief Fades (but does not stop) the audio, from its current volume, to another volume.
		/// @param volume Volume to fade to. Use `-1` for current volume.
		/// @param time Fade duration, in seconds.
		/// @return Reference to self.
		Sound&	fadeTo(float const volume, float const time)				{fade(-1, volume, time); return *this;	}
		/// @brief Fades in (but does not stop) the audio.
		/// @param time Fade duration, in seconds.
		/// @return Reference to self.
		Sound&	fadeIn(float const time)									{fadeTo(1, time); return *this;			}
		/// @brief Fades out (but does not stop) the audio.
		/// @param time Fade duration, in seconds.
		/// @return Reference to self.
		/// @note If audio must stop after fading audio, use `stop` instead.
		Sound&	fadeOut(float const time)									{fadeTo(0, time); return *this;			}

		/// @brief Sets the audio's current playback time.
		/// @param time Playback time, in seconds.
		/// @return Reference to self.
		Sound&	setPlaybackTime(float const time);
		/// @brief Returns the audio's current playback time.
		/// @return Current playback time, in seconds.
		float	getPlaybackTime() const;

		/// @brief Sets the sound's volume.
		/// @param volume Volume to set to.
		Sound&	setVolume(float const volume)	override final;
		/// @brief Returns the sound's volume.
		/// @return Sound volume.
		float	getVolume() const				override final;

		/// @brief Enables/disables 3D spatial audio.
		/// @return Reference to self.
		/// @note Spatial audio is currently unimplemented! Currently does nothing.
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

	/// @brief Analog for `Engine::Sound`.
	using Sound = Engine::Sound;
	/// @brief Analog for `Engine::Group`.
	using Group = Engine::Group;

	/// @brief Analog for `Instance<Sound>`.
	using SoundInstance	= Instance<Sound>;
	/// @brief Analog for `Instance<Group>`.
	using GroupInstance	= Instance<Group>;

	/// @brief Analog for `Handle<Sound>`.
	using SoundHandle	= Handle<Sound>;
	/// @brief Analog for `Handle<Group>`.
	using GroupHandle	= Handle<Group>;
}

#endif // MAKAILIB_AUDIO_ENGINE_H
