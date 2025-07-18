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
		bool exists();

		/// @brief Stops all currently playing sounds.
		/// @param fade Fade-out time in milliseconds
		static void stopAllSounds(usize const fade = 0);
		
		/// @brief Pauses all currently playing sounds.
		static void pauseAllSounds();

		/// @brief Resumes all currently playing sounds.
		static void resumeAllSounds();

		/// @brief Stops the currently-playing music.
		static void stopMusic(usize const fade = 0);
		
		/// @brief Pauses the currently-playing music.
		static void pauseMusic();

		/// @brief Resumes the currently-playing music.
		static void resumeMusic();

		/// @brief Sets the master volume for a source type.
		/// @param volume Volume to set to.
		/// @param type Source type to set master volume for.
		static void setMasterVolume(uchar const volume, SourceType const type);

		/// @brief Gets the master volume for a source type.
		/// @param type Source type to het master volume for.
		static uchar getMasterVolume(SourceType const type);

		/// @brief Sets the music master volume.
		/// @param volume Volume to set to.
		static void setMusicMasterVolume(uchar const volume)	{setMasterVolume(volume, SourceType::ST_MUSIC);	}

		/// @brief Gets the music master volume.
		static uchar getMusicMasterVolume()						{return getMasterVolume(SourceType::ST_MUSIC);	}

		/// @brief Sets the sound master volume.
		/// @param volume Volume to set to.
		static void setSoundMasterVolume(uchar const volume)	{setMasterVolume(volume, SourceType::ST_MUSIC);	}

		/// @brief Gets the sound master volume.
		static uchar getSoundMasterVolume()						{return getMasterVolume(SourceType::ST_MUSIC);	}

		/// @brief Sets the volume of the audio.
		/// @param volume Volume to set.
		void setVolume(uchar const volume);

		/// @brief Returns the current volume of the audio.
		/// @return Current volume.
		int getVolume();

		/// @brief Fades out the current music playing, then fades into this one.
		/// @param fadeOutTime Fade-out time in milliseconds.
		/// @param fadeInTime Fade-in time in milliseconds.
		void switchInto(usize const fadeOutTime, usize const fadeInTime);

		/// @brief Fades out the current music playing, then fades into this one.
		/// @param fadeOutTime Fade-out time in milliseconds.
		/// @param fadeInTime Fade-in time in milliseconds.
		/// @param loops How many times to loop for. `-1` to loop indefinitely.
		void switchInto(usize const fadeOutTime, usize const fadeInTime, int const loops);

		/// @brief Crossfades the current music playing into this one.
		/// @param crossfadeTime Crossfade time in milliseconds.
		void crosfadeInto(usize const crossfadeTime);

		/// @brief Crossfades the current music playing into this one.
		/// @param crossfadeTime Crossfade time in milliseconds.
		void crosfadehInto(usize const crossfadeTime, int const loops);

	protected:
		/// @brief Called when the source is updated.
		void onUpdate() override;

	private:
		/// @brief Sound data assocuated with the source.
		Unique<Content> data;

		/// @brief Whether the source was created.
		bool created = false;
	};
}

#endif // MAKAILIB_AUDIO_PLAYABLE_H
