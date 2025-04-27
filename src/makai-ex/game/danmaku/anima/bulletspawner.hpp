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

		void onObjectRequest(usize const id, usize const count, Reference<AServerObject> const& object, Parameters const& params) override {
			if (auto bullet = object.as<Bullet>()) {
				for (auto const& param: params) {
					if (param.key == ConstHasher::hash("offset")) {
						Vector2 tmp = 0;
						setParameter<Vector2>(object, tmp, param, 0);
						object->trans.position += tmp;
						continue;
					}
					switch (param.key) {
						case (ConstHasher::hash("rotate-sprite")):	setParameter(object, bullet->rotateSprite, param, true);		continue;
						case (ConstHasher::hash("glow-on-spawn")):	setParameter(object, bullet->glowOnSpawn, param, true);			continue;
						case (ConstHasher::hash("dope")):			setParameter(object, bullet->dope, param, true);				continue;
						case (ConstHasher::hash("radius")):			setParameter<Math::Vector2>(object, bullet->radius, param, 1);	continue;
						case (ConstHasher::hash("velocity")):		setParameter<float>(object, bullet->velocity, param, 0);		continue;
						case (ConstHasher::hash("rotation")):		setParameter<float>(object, bullet->rotation, param, 0);		continue;
						case (ConstHasher::hash("damage")):			setParameter<float>(object, bullet->damage, param, 0);			continue;
						case (ConstHasher::hash("glow")):			setParameter<float>(object, bullet->glow, param, 0);			continue;
						case (ConstHasher::hash("auto-decay")):		setParameter(object, bullet->autoDecay, param, true);			continue;
						case (ConstHasher::hash("bouncy")):			setParameter(object, bullet->bouncy, param, true);				continue;
						case (ConstHasher::hash("loopy")):			setParameter(object, bullet->loopy, param, true);				continue;
						case (ConstHasher::hash("grazed")):			setParameter(object, bullet->grazed, param, true);				continue;
					}
					switch (param.key) {
						case (ConstHasher::hash("spread")): {
							float tmp = 0;
							auto const offset = tmp / count;
							setParameter<float>(object, tmp, param, 0);
							bullet->rotation.value += offset * id - (count / 2.0);
							continue;
						}
						case (ConstHasher::hash("offset")): {
							float tmp = 0;
							setParameter<float>(object, tmp, param, 0);
							object->trans.position += Math::angleV2(bullet->rotation.value) * tmp;
							continue;
						}
					}
				}
			}
		}

		void preprocess(float& value, usize const id, ObjectHandle const& object, String const& param) override {
			if (param.empty()) return;
			StringList const params = param.substring(1).split(':');
			if (params.size() < 2 || params.size() > 3) return;
			Reference<AGameObject> target;
			switch (ConstHasher::hash(params.front())) {
				case (ConstHasher::hash("player")):	target = getTargetPlayer(params[1]);
				case (ConstHasher::hash("boss")):	target = getTargetBoss(params[1]);
				case (ConstHasher::hash("enemy")):	target = getTargetEnemy(params[1]);
			}
			float result = 0;
			switch (id) {
				case (ConstHasher::hash("rotation")): result = object->trans.position.angleTo(target->trans.position);
			}
			if (params.size() > 2) {
				try {
					result += toFloat(params[2]);
				} catch (...) {
					throw Error::InvalidValue(
						toString("Invalid value of [" , params[2], "] for number!"),
						CTL_CPP_PRETTY_SOURCE
					);
				}
			}
			value = result;
		}

		virtual Reference<AGameObject>	getTargetPlayer(String const playerID)	= 0;
		virtual Reference<AGameObject>	getTargetBoss(String const bossID)		= 0;
		virtual Reference<AGameObject>	getTargetEnemy(String const enemyID)	= 0;
	};
}

#endif
