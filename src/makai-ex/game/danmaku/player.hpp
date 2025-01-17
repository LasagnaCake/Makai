#ifndef MAKAILIB_EX_GAME_DANMAKU_PLAYER_H
#define MAKAILIB_EX_GAME_DANMAKU_PLAYER_H

#include <makai/makai.hpp>

#include "core.hpp"
#include "bullet.hpp"
#include "laser.hpp"
#include "item.hpp"

#include "../core/controlable.hpp"
#include "../core/sprite.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct PlayerConfig: GameObjectConfig {};

	struct Player: Controllable, AGameObject, AUpdateable {
		struct Velocity {

		};

		Player(PlayerConfig const& cfg):
			AGameObject(cfg) {
				bindmap = Dictionary<String>({
					{"up",		"player/up"		},
					{"down",	"player/down"	},
					{"left",	"player/left"	},
					{"right",	"player/right"	},
					{"shot",	"player/shot"	},
					{"bomb",	"player/bomb"	},
					{"focus",	"player/focus"	}
				});
			}

		constexpr static usize CAN_MOVE		= 1 << 0;
		constexpr static usize CAN_FOCUS	= 1 << 1;
		constexpr static usize CAN_SHOOT	= 1 << 2;
		constexpr static usize CAN_BOMB		= 1 << 3;

		usize actions = CAN_MOVE | CAN_FOCUS | CAN_SHOOT | CAN_BOMB;

		void onUpdate(float delta) override {
			AGameObject::onUpdate(delta);
			if (!active || paused()) return;
		}

		void onUpdate(float delta, App& app) override {
			onUpdate(delta);
			if (!active || paused()) return;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) {
			if (collider.affects.match(CollisionLayer::ENEMY_MASK).overlap()) {}
		}
	};
}

#endif