#ifndef MAKAILIB_EX_GAME_DANMAKU_BOSS_H
#define MAKAILIB_EX_GAME_DANMAKU_BOSS_H

#include "enemy.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct ABoss: AEnemy {
		ABoss(EnemyConfig const& cfg): AEnemy(cfg) {}

		Graph::RadialBar healthBar;

		void onUpdate(float delta) override {
			if (!active) return;
			AGameObject::onUpdate(delta);
			if (paused()) return;
			healthBar.value = health;
			healthBar.max	= maxHealth;
			healthBar.trans.position	= trans.position;
			healthBar.trans.rotation.z	= trans.rotation;
			healthBar.trans.scale		= trans.scale;
		}

		void onUpdate(float delta, App& app) override {
			if (!active) return;
			onUpdate(delta);
			if (paused()) return;
		}
	};
}

#endif