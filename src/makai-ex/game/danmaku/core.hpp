#ifndef MAKAILIB_EX_GAME_DANMAKU_CORE_H
#define MAKAILIB_EX_GAME_DANMAKU_CORE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Danmaku {
	struct Parameter {
		float				from;
		float				to;
		float				by;
		Math::Ease::Mode	ease	= Math::Ease::linear;
		float				current	= 0;
	};

	struct Pause {
		llong	time	= -1;
		bool	enabled	= false;
	};

	struct IObject: Makai::Collision::C2D::Server::ICollider {
	};
}

#endif