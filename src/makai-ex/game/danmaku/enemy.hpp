#ifndef MAKAILIB_EX_GAME_DANMAKU_ENEMY_H
#define MAKAILIB_EX_GAME_DANMAKU_ENEMY_H

#include <makai/makai.hpp>

#include "core.hpp"
#include "bullet.hpp"
#include "laser.hpp"

#include "../core/controlable.hpp"
#include "../core/sprite.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct EnemyConfig: BoundedObjectConfig {

	};

	struct AEnemy: AGameObject, AUpdateable, IDamageable {
		void onUpdate(float delta) override {
			if (!active) return;
			AGameObject::onUpdate(delta);
			if (paused()) return;
		}

		void onUpdate(float delta, App& app) override {
			if (!active) return;
			onUpdate(delta);
			if (paused()) return;
		}
	};
}

#endif