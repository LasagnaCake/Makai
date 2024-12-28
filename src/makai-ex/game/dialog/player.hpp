#ifndef MAKAILIB_EX_GAME_DIALOG_PLAYER_H
#define MAKAILIB_EX_HAME_DIALOG_PLAYER_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog {
	struct Player: IUpdateable, IPlayable {
		using Script = Co::Generator<usize>;

		Player() {isFinished = true;}

		virtual Script script() = 0;

		void onUpdate(float, App&) {
			if (isFinished || paused)	return;
			++counter;
			if (autoplay && waiting())	return;
			if (!shouldAdvanceDialog())	return;
			counter = dialog.next();
			counter = 0;
			if (!dialog) isFinished = true;
		}

		Player& start() override final {
			dialog		= script();
			isFinished	= false;
			counter		= 0;
			play();
		}

		Player& setAutoplay(bool const state) {autoplay = state;}

		Player& stop()	override final	{isFinished = true;	}
		Player& play()	override final	{paused = false;	}
		Player& pause()	override final	{paused = true;		}

		Input::Manager		input;
		Dictionary<String>	bindmap	= Dictionary<String>({
			{"next", "diag-next"},
			{"skip", "diag-skip"}
		});

	private:
		bool shouldAdvanceDialog() {
			return (!waiting()) || (
				input.isButtonJustPressed(bindmap["next"])
			||	input.isButtonDown(bindmap["skip"])
			);
		}

		bool waiting() {
			return counter < delay;
		}

		bool	autoplay	= false;
		usize	counter		= 0;
		usize	delay		= 600;

		Script dialog;
	};
}

#endif