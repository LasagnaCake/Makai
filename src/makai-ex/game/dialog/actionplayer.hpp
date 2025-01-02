#ifndef MAKAILIB_EX_GAME_DIALOG_ACTIONPLAYER_H
#define MAKAILIB_EX_HAME_DIALOG_ACTIONPLAYER_H

#include <makai/makai.hpp>

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief Action program player.
	struct ActionPlayer: IUpdateable, IPlayable {
		/// @brief Program to perform.
		using Program = Co::Generator<usize>;

		/// @brief Empty constructor.
		ActionPlayer() {isFinished = true;}

		/// @brief Program to perform. Must be implemented.
		/// @return Program.
		virtual Program script() = 0;

		/// @brief Executed every update cycle.
		void onUpdate(float, App&) {
			if (isFinished || paused)	return;
			++counter;
			if (starting) [[unlikely]]
				starting = false;
			else [[likely]] {
				if ((!autoplay) && userAdvanced())	next();
				else if (!waiting())				next();
			}
		}

		/// @brief Starts the dialog.
		/// @return Reference to self.
		ActionPlayer& start() override final {
			dialog		= script();
			isFinished	= false;
			counter		= 0;
			return play();
		}

		/// @brief Sets the autoplay state.
		/// @param state Autoplay state.
		/// @return Reference to self.
		ActionPlayer& setAutoplay(bool const state) {autoplay = state; return *this;	}

		/// @brief Stops the dialog.
		/// @return Reference to self.
		ActionPlayer& stop()	override final		{isFinished = true; return *this;	}
		/// @brief Unpauses the dialog.
		/// @return Reference to self.
		ActionPlayer& play()	override final		{paused = false; return *this;		}
		/// @brief Pauses the dialog.
		/// @return Reference to self.
		ActionPlayer& pause()	override final		{paused = true; return *this;		}

		/// @brief Processes the dialog.
		/// @return Reference to self.
		ActionPlayer& next() {
			if (isFinished) return;
			counter = dialog.next();
			if (!dialog) isFinished = true;
			return *this;
		}

		/// @brief Input manager.
		Input::Manager		input;
		/// @brief Input bind map.
		Dictionary<String>	bindmap	= Dictionary<String>({
			{"next", "dialog-next"},
			{"skip", "dialog-skip"}
		});

	private:
		/// @brief Whether the dialog is starting.
		bool starting = true;

		bool userAdvanced() {
			return (!waiting()) || (
				input.isButtonJustPressed(bindmap["next"])
			||	input.isButtonDown(bindmap["skip"])
			);
		}

		bool waiting() {
			return counter < delay;
		}

		/// @brief Whether autoplay is enabled.
		bool	autoplay	= false;
		/// @brief Wait counter.
		usize	counter		= 0;
		/// @brief Max delay to wait for user input.
		usize delay = 600;

		Program dialog;
	};
}

#endif