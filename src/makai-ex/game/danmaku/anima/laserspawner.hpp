#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_LASERSPAWNER_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_LASERSPAWNER_H

#include "serverspawner.hpp"
#include "../laser.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct LaserSpawner: ServerSpawner {
		template<class TLaser = Laser, class TConfig = LaserConfig>
		LaserSpawner(LaserServer<TLaser, TConfig>& server, String const& uniqueName):
			ServerSpawner(server, ConstHasher::hash("laser" + uniqueName)) {}

		void onObjectRequest(usize const id, usize const count, Reference<AServerObject> const& object, Parameters const& params) override {
			if (auto laser = object.as<Laser>()) {
				for (auto const& param: params) {
					switch (param.key) {
						case (ConstHasher::hash("fake-out")):		setParameter(object, laser->fakeOut, param, true);						continue;
						case (ConstHasher::hash("radius")):			setParameter<Math::Vector2>(object, laser->radius, param, 1);			continue;
						case (ConstHasher::hash("head")):			setParameter<Math::Vector2>(object, laser->patch.frame.head, param, 0);	continue;
						case (ConstHasher::hash("body")):			setParameter<Math::Vector2>(object, laser->patch.frame.body, param, 0);	continue;
						case (ConstHasher::hash("tail")):			setParameter<Math::Vector2>(object, laser->patch.frame.tail, param, 0);	continue;
						case (ConstHasher::hash("length")):			setParameter<float>(object, laser->length, param, 0);					continue;
						case (ConstHasher::hash("velocity")):		setParameter<float>(object, laser->velocity, param, 0);					continue;
						case (ConstHasher::hash("rotation")):		setParameter<float>(object, laser->rotation, param, 0);					continue;
						case (ConstHasher::hash("damage")):			setParameter<float>(object, laser->damage, param, 0);					continue;
						case (ConstHasher::hash("auto-decay")):		setParameter(object, laser->autoDecay, param, true);					continue;
						case (ConstHasher::hash("toggle")): {
							usize tmp = 0;
							setParameter<usize>(object, tmp, param, 0);
							if (tmp) laser->toggleTime = tmp;
							laser->toggle(true, !tmp);
						} continue;
						case (ConstHasher::hash("untoggle")): {
							usize tmp = 0;
							setParameter<usize>(object, tmp, param, 0);
							if (tmp) laser->untoggleTime = tmp;
							laser->toggle(false, !tmp);
						} continue;
					}
					switch (param.key) {
						case (ConstHasher::hash("spread")): {
							float tmp = 0;
							auto const offset = tmp / count;
							setParameter<float>(object, tmp, param, 0);
							laser->rotation.value += offset * id - (count / 2.0);
							continue;
						}
						case (ConstHasher::hash("offset")): {
							Vector2 tmp = 0;
							setParameter<Vector2>(object, tmp, param, 0);
							object->trans.position += Math::angleV2(laser->rotation.value) * tmp;
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

		bool preprocess(Math::Vector2& value, usize const id, ObjectHandle const& object, String const& param) override {
			if (ServerSpawner::preprocess(value, id, object, param)) return true;
			if (param.empty()) return false;
			StringList const params = param.split(':');
			if (params.size() < 1) return false;
			usize const type	= ConstHasher::hash(params[0]);
			String const name	= params.size() < 2 ? "" : params[1];
			Reference<AGameObject> target = ITargetsObjects::getTarget(type, name);
			Math::Vector2 result = 0;
			if (target) switch (id) {
				case (ConstHasher::hash("position")): result = target->trans.position;
			}
			if (params.size() > 2) {
				try {
					result += convert<2>(params[2], 0);
				} catch (...) {
					throw Error::InvalidValue(
						toString("Invalid value of [" , params[2], "] for number!"),
						CTL_CPP_PRETTY_SOURCE
					);
				}
			}
			value = result;
		}

		bool preprocess(float& value, usize const id, ObjectHandle const& object, String const& param) override {
			if (ServerSpawner::preprocess(value, id, object, param)) return true;
			if (param.empty()) return false;
			StringList const params = param.split(':');
			if (params.size() < 1) return false;
			usize const type	= ConstHasher::hash(params[0]);
			String const name	= params.size() < 2 ? "" : params[1];
			Reference<AGameObject> target = ITargetsObjects::getTarget(type, name);
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
