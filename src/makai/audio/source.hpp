#ifndef MAKAILIB_AUDIO_PLAYABLE_H
#define MAKAILIB_AUDIO_PLAYABLE_H

#include "../compat/ctl.hpp"

#include "../core/extern.hpp"
#include "../data/encdec.hpp"

/// @brief Audio facilities.
namespace Makai::Audio {
	/// @brief Playable audio source.
	class Source;

	/// @brief Audio periodic event.
	using APeriodicSource = APeriodic<Source>;

	/// @brief Playable audio source.
	class Source: APeriodicSource {
	public:
		/// @brief Audio source content.
		struct Content;
		
		/// @brief Audio source world.
		struct World {
			/// @brief World size.
			Vector2 size;
		};

		/// @brief Audio source listener.
		struct Listener {
			/// @brief Listener position.
			Vector2 position;
		};

		/// @brief Audio source type
		enum class SourceType {
			ST_SOUND,
			ST_MUSIC
		};

		/// @brief Default constructor.
		Source();

		/// @brief Creates the audio object from an audio file.
		/// @param path Path to audio file.
		/// @param type Source type.
		Source(String const& path, SourceType const type);

		/// @brief Destructor.
		virtual ~Source();

		/// @brief Creates the source from an audio file.
		/// @param path Path to audio file.
		/// @param type Source type.
		void create(String const& path, SourceType const type);

		/// @brief Creates the source from an audio file, and sets it as a sound source.
		/// @param path Path to audio file.
		void createSound(String const& path) {create(path, SourceType::ST_SOUND);}

		/// @brief Creates the source from an audio file, and sets it as a music source.
		/// @param path Path to audio file.
		void createMusic(String const& path) {create(path, SourceType::ST_MUSIC);}

		/// @brief Sets the source's type.
		/// @param type Type to set source to.
		void setType(SourceType const type);

		/// @brief Destroys the source.
		void destroy();

		/// @brief Called when playback is finished.
		virtual void onPlaybackEnd()	{}
		/// @brief Called when playback is started.
		virtual void onPlaybackStart()	{}

		/// @brief Returns whether the source exists.
		/// @return Whether source exists.
		bool exists() const;
		/// @brief Returns whether the source is currently playing.
		/// @return Whether source is playing.
		bool playing() const;
		/// @brief Returns whether the source is a music source.
		/// @return Whether source is a music source.
		bool isMusic() const;

		/// @brief Stops all currently playing sources of a given type.
		/// @param fadeOutTime Fade-out time in milliseconds.
		/// @param type Source type to execute operation on.
		static void masterStop(uint const fadeOutTime, SourceType const type);
		/// @brief Pauses all currently playing sources of a given type.
		/// @param type Source type to execute operation on.
		static void masterPause(SourceType const type);
		/// @brief Unpauses all currently playing sources of a given type.
		/// @param type Source type to execute operation on.
		static void masterUnpause(SourceType const type);
		/// @brief Sets the master volume for a source type.
		/// @param volume Volume to set to.
		/// @param type Source type to set master volume for.
		static void setMasterVolume(float const volume, SourceType const type);
		/// @brief Gets the master volume for a source type.
		/// @param type Source type to het master volume for.
		static float getMasterVolume(SourceType const type);

		/// @brief Stops all currently playing sounds.
		/// @param fadeOutTime Fade-out time in milliseconds.
		inline static void stopAllSounds(uint const fadeOutTime = 0)	{masterStop(fadeOutTime, SourceType::ST_SOUND);	}
		/// @brief Pauses all currently playing sounds.
		inline static void pauseAllSounds()								{masterPause(SourceType::ST_SOUND);				}
		/// @brief Resumes all currently playing sounds.
		inline static void resumeAllSounds()							{masterUnpause(SourceType::ST_SOUND);				}
		/// @brief Sets the music master volume.
		/// @param volume Volume to set to.
		inline static void setMusicMasterVolume(float const volume)		{setMasterVolume(volume, SourceType::ST_MUSIC);	}
		/// @brief Gets the music master volume.
		inline static float getMusicMasterVolume()						{return getMasterVolume(SourceType::ST_MUSIC);	}

		/// @brief Stops all currently-playing music.
		/// @param fadeOutTime Fade-out time in milliseconds.
		inline static void stopAllMusic(uint const fadeOutTime = 0)	{masterStop(fadeOutTime, SourceType::ST_MUSIC);	}
		/// @brief Pauses all currently-playing music.
		inline static void pauseAllMusic()							{masterPause(SourceType::ST_MUSIC);				}
		/// @brief Resumes all currently-playing music.
		inline static void resumeAllMusic()							{masterUnpause(SourceType::ST_MUSIC);				}
		/// @brief Sets the sound master volume.
		/// @param volume Volume to set to.
		inline static void setSoundMasterVolume(float const volume)	{setMasterVolume(volume, SourceType::ST_SOUND);	}
		/// @brief Gets the sound master volume.
		inline static float getSoundMasterVolume()					{return getMasterVolume(SourceType::ST_SOUND);	}

		/// @brief Plays the source.
		/// @param loops How many times to loop for. `-1` to loop indefinitely. By default, it is zero.
		/// @param fadeInTime Fade-in time in milliseconds. By default, it is zero.
		/// @param force Whether to force playing the source from the beginning, if already playing. By default, it is `false`.
		void play(
			int const	loops		= 0,
			uint const	fadeInTime	= 0,
			bool const	force		= false
		);

		/// @brief Plays the source. Starts the source only one time in the current logic cycle.
		/// @param loops How many times to loop for. `-1` to loop indefinitely. By default, it is zero.
		/// @param fadeInTime Fade-in time in milliseconds. By default, it is zero.
		/// @param force Whether to force playing the source from the beginning, if already playing. By default, it is `false`.
		void playOnceThisFrame(
			int const	loops		= 0,
			uint const	fadeInTime	= 0,
			bool const	force		= false
		);

		/// @brief Plays the source, then waits a given number of cycles before it can be played again.
		/// @param loops How many times to loop for. `-1` to loop indefinitely. By default, it is zero.
		/// @param fadeInTime Fade-in time in milliseconds. By default, it is zero.
		/// @param force Whether to force playing the source from the beginning, if already playing. By default, it is `false`.
		/// @param cycles How many cycles to wait before the source can be played again. By default, it is zero.
		void playOnceAndWait(
			int const	loops		= 0,
			uint const	fadeInTime	= 0,
			bool const	force		= false,
			usize const	cycles		= 0
		);

		/// @brief Stops the source.
		/// @param fadeOutTime Fade-out time in milliseconds.
		void stop(uint const fadeOutTime = 0);

		/// @brief Pauses the source, if it is playing.
		void pause();

		/// @brief Unpauses the source, if it was playing.
		void unpause();

		/// @brief Cross-fades the current music playing into this one.
		/// @param crossfadeTime Cross-fade time in milliseconds.
		/// @param loops How many times to loop for. `-1` to loop indefinitely. By default, it is zero.
		void crossFadeInto(uint const crossFadeTime, int const loops = 0);

		/// @brief Updates the audio source subsystem.
		static void process();

		/// @brief Sets whether the audio source is "spatial".
		/// @param dimensional Whether audio source is "spatial".
		/// @note
		///		Simulates a "pseudo-spatial" audio implementation. Stereo-only.
		///		For mono channels, it only changes the sound volume.
		void setSpatial(bool const spatial = true);

		/// @brief Source position in the audio world.
		Vector2 position;

		/// @brief Source volume.
		float volume = 1;

		/// @brief Audio source listener.
		inline static Listener listener	= {.position	= {0, 0}};
		/// @brief Audio source world.
		inline static World world		= {.size		= {1, 1}};

	protected:
		/// @brief Called when the source is updated.
		void onUpdate() override;

	private:
		void updateVolume();

		/// @brief Sound data assocuated with the source.
		Unique<Content> data;

		/// @brief Whether the source was created.
		bool created = false;

		/// @brief Whether the source was playing in the previous cycle.
		bool wasPlaying = false;

		/// @brief Wether the audio source is spatial.
		bool spatial = false;

		/// @brief Time to wait before the source can be played again.
		usize cooldown = 0;
	};
}

#endif // MAKAILIB_AUDIO_PLAYABLE_H
