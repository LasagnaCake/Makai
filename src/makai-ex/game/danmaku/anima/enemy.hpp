#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_ENEMY_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_ENEMY_H

#include "../enemy.hpp"
#include "../../anima/simpleengine.hpp"
#include "../../core/registry.hpp"
#include "decode.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct Enemy;

	using EnemyRegistry = Registry<Enemy, ConstHasher::hash("Danmaku::Anima::Enemy")>;

	struct AAnimaEnemy: AEnemy, EnemyRegistry::Member, private AVM::SimpleEngine {
		Makai::Graph::Renderable		mesh;
		Makai::Ex::Game::SpriteInstance	sprite;

		AAnimaEnemy(EnemyConfig const& cfg): AEnemy(cfg) {
			trans.position = playfield.center;
			sprite = mesh.createReference<Makai::Ex::Game::Sprite>();
			mesh.setRenderLayer(Danmaku::Render::Layer::ENEMY1_LAYER);
			movement.setManual();
		}

		void onUpdate(float delta, Makai::App& app) override {
			if (!active) return;
			AEnemy::onUpdate(delta, app);
			updateMesh();
			if (AEnemy::paused()) return;
			SimpleEngine::process();
			movement.onUpdate(1);
			acceleration.onUpdate(1);
			if (SimpleEngine::paused) return;
			if (!movement.finished()) {
				direction = (movement.value() - trans.position).normalized();
				trans.position = movement.value();
			} else {
				if (!acceleration.finished())
					speed = acceleration.value();
				trans.position += speed;
				direction = speed.normalized();
			}
			updateMesh();
		}

		void onDeath() override {
			queueDestroy();
		}

		AAnimaEnemy& moveTo(Math::Vector2 const& position, usize const time) {
			movement.reinterpolateTo(position, time);
			return *this;
		}

		AAnimaEnemy& moveTo(Math::Vector2 const& position, usize const time, Math::Ease::Mode const& mode) {
			movement.easeMode = mode;
			moveTo(position, time);
			return *this;
		}

		Math::Vector2 speed;
		
	protected:
		Math::Vector2 direction;

		void updateMesh() {
			mesh.trans.position		= trans.position;
			mesh.trans.rotation.z	= trans.rotation;
			mesh.trans.scale		= trans.scale;
		}

		bool execute(usize const name, Parameters const& params) override {
			switch (name) {
				case (ConstHasher::hash("move-to")):		return true;
				case (ConstHasher::hash("accelerate-to")):	return true;
				case (ConstHasher::hash("scale")):			return true;
				case (ConstHasher::hash("radius")):			return true;
				case (ConstHasher::hash("velocity")):		return true;
			}
			return false;
		}

		Tween<Math::Vector2> movement;
		Tween<Math::Vector2> acceleration;

	private:
		
	};
}

#endif