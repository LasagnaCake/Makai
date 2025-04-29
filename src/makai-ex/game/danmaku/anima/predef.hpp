#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_PREDEF_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_PREDEF_H

#include "../enemy.hpp"
#include "../boss.hpp"
#include "../player.hpp"
#include "../../core/registry.hpp"
#include "decode.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct AAnimaEnemy;
	using EnemyRegistry		= Registry<AAnimaEnemy, ConstHasher::hash("Danmaku::Anima::Enemy")>;
	
	struct AAnimaBoss;
	using BossRegistry		= Registry<AAnimaBoss, ConstHasher::hash("Danmaku::Anima::Boss")>;
	
	struct AAnimaPlayer;
	using PlayerRegistry	= Registry<AAnimaPlayer, ConstHasher::hash("Danmaku::Anima::Player")>;
}

#endif