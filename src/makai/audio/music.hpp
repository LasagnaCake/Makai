#ifndef MAKAILIB_AUDIO_MUSIC_H
#define MAKAILIB_AUDIO_MUSIC_H

#include "core.hpp"
#include "source.hpp"

/// @brief Audio facilities.
namespace Makai::Audio {
	/// @brief Music source.
	class Music: public ISource {
	public:
		/// @brief File metadata.
		struct Metadata {
			String	title;
			String	artist;
			String	album;
			String	copyright;
		};

		using ISource::ISource;

		/// @brief Destructor.
		virtual ~Music() final;
		
		/// @brief Plays the music.
		/// @param loops How many times to loop for. `-1` to loop indefinitely. By default, it is zero.
		/// @param fadeInTime Fade-in time in milliseconds. By default, it is zero.
		void play(int const loops = 0, usize const fadeInTime = 0);

		/// @brief Returns the music title.
		/// @return Music title.
		String getTitle();

		/// @brief Returns the file metadate.
		/// @return File metadata.
		Metadata getMetadata();

		/// @brief Fades out the current music playing, then fades into this one.
		/// @param fadeOutTime Fade-out time in milliseconds.
		/// @param fadeInTime Fade-in time in milliseconds.
		void switchInto(usize const fadeOutTime, usize const fadeInTime);

		/// @brief Fades out the current music playing, then fades into this one.
		/// @param fadeOutTime Fade-out time in milliseconds.
		/// @param fadeInTime Fade-in time in milliseconds.
		/// @param loops How many times to loop for. `-1` to loop indefinitely.
		void switchInto(usize const fadeOutTime, usize const fadeInTime, int const loops);

		/// @brief Queues the music to be played next.
		void queueMusic();

		/// @brief Returns the underlying music source.
		/// @return Underlying music source.
		inline Extern::Resource getSource();

		/// @brief
		///		Updates the music subsystem.
		///		Not required to call every cycle, as the program already handles it.
		static void		update();
		/// @brief Stops the currently playing music.
		/// @param fade Fade-out time in milliseconds.
		static void		stop(usize const fade = 0);
		/// @brief Pauses the currently playing music.
		static void		pause();
		/// @brief Resumes the currently playing music.
		static void		resume();
		/// @brief Sets the music volume.
		/// @param volume Volume to set.
		static void		setVolume(schar const volume);
		/// @brief Returns the current music volume.
		/// @return Current music volume.
		static schar	getVolume();

	protected:
		/// @brief Called when the music is created.
		/// @param data Music source.
		void onCreate(Extern::Resource const& data) final override;

		/// @brief Called when the music is destroyed.
		void onDestroy() final override;

		/// @brief Called when the music is updated.
		void onUpdate() final override;

	private:
		/// @brief Queued fade-in time.
		usize	fadeInTime	= 0;
		/// @brief Queued loop count.
		int		loops		= 0;

		/// @brief Callback called when queued music is to be played.
		AudioCallback const onQueue = [&] {play(loops, fadeInTime);};

		/// @brief Music source.
		Extern::Resource source = NULL;
	};
}

#endif // MAKAILIB_AUDIO_MUSIC_H
