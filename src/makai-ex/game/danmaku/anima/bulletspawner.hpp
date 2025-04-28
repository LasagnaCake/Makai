#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_BULLETSPAWNER_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_BULLETSPAWNER_H

#include "serverspawner.hpp"
#include "../bullet.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct BulletSpawner: ServerSpawner<> {
		template<class TBullet = Bullet, class TConfig = BulletConfig>
		BulletSpawner(BulletServer<TBullet, TConfig>& server, String const& uniqueName):
			ServerSpawner(server, ConstHasher::hash("bullet" + uniqueName)) {}

		void onObjectRequest(usize const id, usize const count, Reference<AServerObject> const& object, Parameters const& params) override {
			if (auto bullet = object.as<Bullet>()) {
				for (auto const& param: params) {
					switch (param.key) {
						case (ConstHasher::hash("rotate-sprite")):	setParameter(object, bullet->rotateSprite, param, true);				continue;
						case (ConstHasher::hash("glow-on-spawn")):	setParameter(object, bullet->glowOnSpawn, param, true);					continue;
						case (ConstHasher::hash("dope")):			setParameter(object, bullet->dope, param, true);						continue;
						case (ConstHasher::hash("radius")):			setParameter<Math::Vector2>(object, bullet->radius, param, 1);			continue;
						case (ConstHasher::hash("velocity")):		setParameter<float>(object, bullet->velocity, param, 0);				continue;
						case (ConstHasher::hash("rotation")):		setParameter<float>(object, bullet->rotation, param, 0);				continue;
						case (ConstHasher::hash("damage")):			setParameter<float>(object, bullet->damage, param, 0);					continue;
						case (ConstHasher::hash("glow")):			setParameter<float>(object, bullet->glow, param, 0);					continue;
						case (ConstHasher::hash("auto-decay")):		setParameter(object, bullet->autoDecay, param, true);					continue;
						case (ConstHasher::hash("bouncy")):			setParameter(object, bullet->bouncy, param, true);						continue;
						case (ConstHasher::hash("loopy")):			setParameter(object, bullet->loopy, param, true);						continue;
						case (ConstHasher::hash("grazed")):			setParameter(object, bullet->grazed, param, true);						continue;
						case (ConstHasher::hash("sprite")):			setParameter<Math::Vector2>(object, bullet->sprite.frame, param, 0);	continue;
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
							Vector2 tmp = 0;
							setParameter<Vector2>(object, tmp, param, 0);
							object->trans.position += Math::angleV2(bullet->rotation.value) * tmp;
							continue;
						}
						case (ConstHasher::hash("wait")): {
							ssize tmp = 0;
							setParameter<ssize>(object, tmp, param, 0);
							if (tmp) object->pause = {tmp, true};
							continue;
						}
					}
				}
			}
		}

		bool preprocess(float& value, usize const id, ObjectHandle const& object, String const& param) override {
			if (ServerSpawner::preprocess(value, id, object, param)) return true;
			if (param.empty()) return false;
			StringList const params = param.split(':');
			if (params.size() < 1) return false;
			usize const type	= ConstHasher::hash(params[0]);
			String const name	= params.size() < 2 ? "" : params[1];
			Reference<AGameObject> target = ITargetsObjects::getTarget(type, name);
			if (!target) target = IParented::getTarget(type, name);
			float result = 0;
			if (target) switch (id) {
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
	};
}

#endif
