#ifndef MAKAILIB_AUDIO_SOUND_H
#define MAKAILIB_AUDIO_SOUND_H

#include "../compat/ctl.hpp"

#include "core.hpp"
#include "source.hpp"

/// @brief Audio facilities.
namespace Makai::Audio {
	/// @brief Sound source.
	class Sound: public ISource {
	public:
		using ISource::ISource;

		/// @brief Destructor.
		virtual ~Sound() final;

		/// @brief Stops all currently playing sounds.
		/// @param fade Fade-out time in milliseconds
		static void stopAll(usize const fade = 0);
		
		/// @brief Pauses all currently playing sounds.
		static void pauseAll();

		/// @brief Resumes all currently playing sounds.
		static void resumeAll();

		/// @brief Sets the master volume.
		static void setMasterVolume(schar const volume);

		/// @brief Gets the master volume.
		static schar getMasterVolume();

		/// @brief Plays the sound. Ignores all other behaviour.
		/// @param loops How many times to loop for. `-1` to loop indefinitely. By default, it is zero.
		/// @param fadeInTime Fade-in time in milliseconds. By default, it is zero.
		/// @param force Whether to force playing the sound from the beginning, if already playing. By default, it is `false`.
		void play(
			int const	loops		= 0,
			usize const	fadeInTime	= 0,
			bool const	force		= false
		);

		/// @brief Plays the sound. Starts the sound only one time in the current logic cycle.
		/// @param loops How many times to loop for. `-1` to loop indefinitely. By default, it is zero.
		/// @param fadeInTime Fade-in time in milliseconds. By default, it is zero.
		/// @param force Whether to force playing the sound from the beginning, if already playing. By default, it is `false`.
		void playOnceThisFrame(
			int const	loops		= 0,
			usize const	fadeInTime	= 0,
			bool const	force		= false
		);

		/// @brief Plays the sound, then waits a given number of cycles before it can be played again.
		/// @param loops How many times to loop for. `-1` to loop indefinitely. By default, it is zero.
		/// @param fadeInTime Fade-in time in milliseconds. By default, it is zero.
		/// @param force Whether to force playing the sound from the beginning, if already playing. By default, it is `false`.
		/// @param cycles How many cycles to wait before the sound can be played again. By default, it is zero.
		void playOnceAndWait(
			int const	loops		= 0,
			usize const	fadeInTime	= 0,
			bool const	force		= false,
			usize const	cycles		= 0
		);

		/// @brief Stops the sound.
		/// @param fadeOutTime Fade-out time in milliseconds.
		void stop(usize const fadeOutTime = 0);

		/// @brief Pauses the sound.
		void pause();

		/// @brief Resumes the sound.
		void resume();

		/// @brief Sets the volume of the sound.
		/// @param volume Volume to set.
		void setVolume(uchar const volume);

		/// @brief Returns the current volume of the sound.
		/// @return Current volume.
		int getVolume();

	protected:
		/// @brief Called when the music is created.
		/// @param data Music source.
		void onCreate(Extern::Resource const& data) final override;

		/// @brief Called when the music is destroyed.
		void onDestroy() final override;

		/// @brief Called when the music is updated.
		void onUpdate() final override;

	private:
		/// @brief Whether the sound can be played this frame.
		bool canPlayThisFrame = true;

		/// @brief Counter to wait until sound can be played again.
		usize delay = 0;
 
		int getChannel();

		bool active();

		/// @brief Channel sound is being played in.
		int					channel = -1;
		/// @brief Sound source.
		Extern::Resource	source	= NULL;
	};
}

#endif // MAKAILIB_AUDIO_SOUND_H
