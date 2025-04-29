#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_ITEMSPAWNER_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_ITEMSPAWNER_H

#include "serverspawner.hpp"
#include "../item.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct BulletSpawner: ServerSpawner {
		template<class TItem = Item, class TConfig = ItemConfig>
		BulletSpawner(ItemServer<TItem, TConfig>& server, String const& uniqueName):
			ServerSpawner(server, ConstHasher::hash("item" + uniqueName)) {}

		float spread	= 0;
		float angle		= 0;

		void onObjectRequest(usize const id, usize const count, Reference<AServerObject> const& object, Parameters const& params) override {
			if (auto item = object.as<Item>()) {
				for (auto const& param: params) {
					switch (param.key) {
						case (ConstHasher::hash("rotate-sprite")):	setParameter(object, item->rotateSprite, param, true);					continue;
						case (ConstHasher::hash("glow-on-spawn")):	setParameter(object, item->glowOnSpawn, param, true);					continue;
						case (ConstHasher::hash("dope")):			setParameter(object, item->dope, param, true);							continue;
						case (ConstHasher::hash("radius")):			setParameter<Math::Vector2>(object, item->radius, param, 1);			continue;
						case (ConstHasher::hash("gravity")):		setParameter<Math::Vector2>(object, item->gravity, param, 0);			continue;
						case (ConstHasher::hash("max-velocity")):	setParameter<Math::Vector2>(object, item->terminalVelocity, param, 0);	continue;
						case (ConstHasher::hash("glow")):			setParameter<float>(object, item->glow, param, 0);						continue;
						case (ConstHasher::hash("jumpy")):			setParameter(object, item->jumpy, param, true);							continue;
						case (ConstHasher::hash("id")):				setParameter<usize>(object, item->id, param, 0);						continue;
						case (ConstHasher::hash("value")):			setParameter<usize>(object, item->value, param, 1);						continue;
						case (ConstHasher::hash("sprite")):			setParameter<Math::Vector2>(object, item->sprite.frame, param, 1);		continue;
						case (ConstHasher::hash("spread")):			setParameter<float>(object, spread, param, TAU);						continue;
						case (ConstHasher::hash("angle")):			setParameter<float>(object, angle, param, 0);							continue;
					}
					switch (param.key) {
						case (ConstHasher::hash("offset")): {
							Vector2 tmp = 0;
							setParameter<Math::Vector2>(object, tmp, param, 0);
							auto const offset = spread / count;
							object->trans.position += Math::angleV2(offset * id - (count / 2.0) + angle) * tmp;
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
