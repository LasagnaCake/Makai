#ifndef MAKAILIB_EX_GAME_DIALOG_ACTIONPLAYER_H
#define MAKAILIB_EX_HAME_DIALOG_ACTIONPLAYER_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog {
	struct ActionPlayer: IUpdateable, IPlayable {
		using Script = Co::Generator<usize>;

		ActionPlayer() {isFinished = true;}

		virtual Script script() = 0;

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

		ActionPlayer& start() override final {
			dialog		= script();
			isFinished	= false;
			counter		= 0;
			return play();
		}

		ActionPlayer& setAutoplay(bool const state) {autoplay = state; return *this;	}

		ActionPlayer& stop()	override final		{isFinished = true; return *this;	}
		ActionPlayer& play()	override final		{paused = false; return *this;		}
		ActionPlayer& pause()	override final		{paused = true; return *this;		}

		ActionPlayer& next() {
			if (isFinished) return;
			counter = dialog.next();
			if (!dialog) isFinished = true;
			return *this;
		}

		Input::Manager		input;
		Dictionary<String>	bindmap	= Dictionary<String>({
			{"next", "dialog-next"},
			{"skip", "dialog-skip"}
		});

	private:
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

		bool	autoplay	= false;
		usize	counter		= 0;
		usize	delay		= 600;

		Script dialog;
	};
}

#endif