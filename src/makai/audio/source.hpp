#ifndef MAKAILIB_AUDIO_PLAYABLE_H
#define MAKAILIB_AUDIO_PLAYABLE_H

#include "../compat/ctl.hpp"

#include "../core/extern.hpp"
#include "../data/encdec.hpp"

/// @brief Audio facilities.
namespace Makai::Audio {
	/// @brief Playable source abstract class.
	class APlayable;

	/// @brief Audio periodic event.
	using APeriodicAudio = APeriodic<APlayable>;

	/// @brief Playable audio source.
	class Source: APeriodicAudio {
	public:
		/// @brief Audio source content.
		struct Content;

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
		static void stopAllSounds(uint const fadeOutTime = 0)	{masterStop(fadeOutTime, SourceType::ST_SOUND);	}
		/// @brief Pauses all currently playing sounds.
		static void pauseAllSounds()							{masterPause(SourceType::ST_SOUND);				}
		/// @brief Resumes all currently playing sounds.
		static void resumeAllSounds()							{masterUnpause(SourceType::ST_SOUND);				}

		/// @brief Stops the currently-playing music.
		/// @param fadeOutTime Fade-out time in milliseconds.
		static void stopMusic(uint const fadeOutTime = 0)		{masterStop(fadeOutTime, SourceType::ST_MUSIC);	}
		/// @brief Pauses the currently-playing music.
		static void pauseMusic()								{masterPause(SourceType::ST_MUSIC);				}
		/// @brief Resumes the currently-playing music.
		static void resumeMusic()								{masterUnpause(SourceType::ST_MUSIC);				}

		/// @brief Sets the music master volume.
		/// @param volume Volume to set to.
		static void setMusicMasterVolume(float const volume)	{setMasterVolume(volume, SourceType::ST_MUSIC);	}

		/// @brief Gets the music master volume.
		static float getMusicMasterVolume()						{return getMasterVolume(SourceType::ST_MUSIC);	}

		/// @brief Sets the sound master volume.
		/// @param volume Volume to set to.
		static void setSoundMasterVolume(float const volume)	{setMasterVolume(volume, SourceType::ST_SOUND);	}

		/// @brief Gets the sound master volume.
		static float getSoundMasterVolume()						{return getMasterVolume(SourceType::ST_SOUND);	}

		/// @brief Queues the music for playback.
		void queueMusic(uint const fadeInTime, int const loops);

		/// @brief Sets the volume of the source.
		/// @param volume Volume to set.
		void setVolume(float const volume);

		/// @brief Returns the current volume of the source.
		/// @return Current volume.
		float getVolume() const;

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

		/// @brief Pauses the source.
		void pause();

		/// @brief Fades out the current music playing, then fades into this one.
		/// @param fadeOutTime Fade-out time in milliseconds.
		/// @param fadeInTime Fade-in time in milliseconds.
		void switchInto(uint const fadeOutTime, uint const fadeInTime) {
			switchInto(fadeOutTime, fadeInTime, 0);
		}

		/// @brief Fades out the current music playing, then fades into this one.
		/// @param fadeOutTime Fade-out time in milliseconds.
		/// @param fadeInTime Fade-in time in milliseconds.
		/// @param loops How many times to loop for. `-1` to loop indefinitely.
		void switchInto(uint const fadeOutTime, uint const fadeInTime, int const loops) {
			stopMusic(fadeOutTime);
			queueMusic(fadeInTime, loops);
		}

		/// @brief Cross-fades the current music playing into this one.
		/// @param crossfadeTime Cross-fade time in milliseconds.
		void crossFadeInto(uint const crossFadeTime);

		/// @brief Cross-fades the current music playing into this one.
		/// @param crossfadeTime Cross-fade time in milliseconds.
		void crossFadeInto(uint const crossFadeTime, int const loops);

		/// @brief Updates the audio source subsystem.
		static void process();

	protected:
		/// @brief Called when the source is updated.
		void onUpdate() override;

	private:
		/// @brief Sound data assocuated with the source.
		Unique<Content> data;

		/// @brief Whether the source was created.
		bool created = false;

		/// @brief Time to wait before the source can be played again.
		usize cooldown = 0;
	};
}

#endif // MAKAILIB_AUDIO_PLAYABLE_H
