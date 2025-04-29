#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_INTERFACES_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_INTERFACES_H

#include "../player.hpp"
#include "../boss.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct IObjectSolver {
		virtual ~IObjectSolver() {}

		Reference<AGameObject> getTarget(usize const type, String const name) {
			switch (type) {
				case (ConstHasher::hash("@self")):		return getSelf();
				case (ConstHasher::hash("@player")):	if (name.empty()) return nullptr; return getTargetPlayer(name);
				case (ConstHasher::hash("@boss")):		if (name.empty()) return nullptr; return getTargetBoss(name);
				case (ConstHasher::hash("@enemy")):		if (name.empty()) return nullptr; return getTargetEnemy(name);
			}
			return nullptr;
		}

		virtual Reference<AGameObject>	getSelf()							{return nullptr;}
		virtual Reference<AGameObject>	getTargetPlayer(String const name)	{return nullptr;}
		virtual Reference<AGameObject>	getTargetBoss(String const name)	{return nullptr;}
		virtual Reference<AGameObject>	getTargetEnemy(String const name)	{return nullptr;}
	};
}

#endif