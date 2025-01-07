#ifndef MAKAILIB_EX_GAME_DANMAKU_CORE_H
#define MAKAILIB_EX_GAME_DANMAKU_CORE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Danmaku {
	using CollisionServer = Makai::Collision::C2D::Server;

	struct Parameter {
		float				from;
		float				to;
		float				by;
		Math::Ease::Mode	ease	= Math::Ease::linear;
		float				current	= 0;
	};

	struct PauseState {
		llong	time	= -1;
		bool	enabled	= false;
	};

	struct GameObject: CollisionServer::ICollider, Makai::IUpdateable {
		Makai::Co::Promise<usize, true> task;

		PauseState pause;

		Math::Transform2D trans;

		void onUpdate(float, Makai::App&) override {
			if (pause.enabled && pause.time > 0) {
				--pause.time;
				return;
			}
			pause.enabled = false;
			if (delay > 0) {
				--delay;
				return;
			}
			while (!delay && task)
				delay = task.next();
		}

	private:
		usize delay = 0;
	};
}

#endif