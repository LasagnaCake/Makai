#ifndef MAKAILIB_EX_GAME_DANMAKU_BOSS_H
#define MAKAILIB_EX_GAME_DANMAKU_BOSS_H

#include "enemy.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct ABoss: AEnemy {
		ABoss(EnemyConfig const& cfg): AEnemy(cfg) {}
	};
}

#endif