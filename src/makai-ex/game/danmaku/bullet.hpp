#ifndef MAKAILIB_EX_GAME_DANMAKU_BULLET_H
#define MAKAILIB_EX_GAME_DANMAKU_BULLET_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct Bullet: AttackObject {
		Bullet(Server& server): server(server) {}

	private:
		Server& server;
	};
}

#endif