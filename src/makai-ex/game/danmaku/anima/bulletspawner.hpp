#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_BULLETSPAWNER_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_BULLETSPAWNER_H

#include "serverspawner.hpp"
#include "../bullet.hpp"
#include "../player.hpp"
#include "../boss.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct ABulletSpawner: ServerSpawner {
		template<class TBullet = Bullet, class TConfig = BulletConfig>
		ABulletSpawner(BulletServer<TBullet, TConfig>& server, String const& uniqueName):
			ServerSpawner(server, ConstHasher::hash("bullet:" + uniqueName)) {}

		void onObjectRequest(Reference<AServerObject> const& object, Parameters const& params) override {
			if (auto bullet = object.as<Bullet>()) {

			}
		}

		virtual Reference<APlayer>	getTargetPlayer(usize const playerID)	= 0;
		virtual Reference<ABoss>	getTargetBoss(usize const bossID)		= 0;
		virtual Reference<AEnemy>	getTargetEnemy(usize const enemyID)		= 0;
	};
}

#endif
