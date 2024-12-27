#ifndef MAKAILIB_EX_GAME_DIALOG_PLAYER_H
#define MAKAILIB_EX_HAME_DIALOG_PLAYER_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog {
	struct Player: IUpdateable, IPlayable {
		using Script = Co::Routine;

		struct Waiter: Co::Consumer {
			Waiter(usize& delay, usize const wait): delay(delay), wait(wait) {}
		private:
			usize&	delay;
			usize	wait;
			void onEnter() override final	{CTL::swap(delay, wait);}
			void onExit() override final	{CTL::swap(delay, wait);}
		};

		Player() {isFinished = true;}

		virtual Script script() = 0;

		void onUpdate(float, App&) {
			if (isFinished || paused) return;
			if (autotimer++ < delay) return;
			dialog.process();
			autotimer = 0;
			if (!dialog) isFinished = true;
		}

		Player& start() override final {
			dialog = script();
			isFinished = false;
			play();
		}

		Player& setAutoplay(bool const state)	{autoplay = state;		}
		Waiter wait(usize const time)			{return {delay, time};	}

		Player& stop()	override final	{isFinished = true;	}
		Player& play()	override final	{paused = false;	}
		Player& pause()	override final	{paused = true;		}

		usize delay = 600;

		Input::Manager input;

	private:
		bool autoplay	= false;
		usize autotimer	= 0;

		Script dialog;
	};
}

#endif