#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_ENEMY_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_ENEMY_H

#include "../enemy.hpp"
#include "../../anima/simpleengine.hpp"
#include "../../core/registry.hpp"
#include "interfaces.hpp"
#include "decode.hpp"
#include "predef.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct AAnimaEnemy: AEnemy, EnemyRegistry::Member, private AVM::SimpleEngine {
		Makai::Graph::Renderable		mesh;
		Makai::Ex::Game::SpriteInstance	sprite;

		IObjectSolver& solver;

		AAnimaEnemy(EnemyConfig const& cfg, IObjectSolver& solver): AEnemy(cfg), solver(solver) {
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
				trans.position += Math::angleV2(angle) * speed * delta;
				direction = Math::angleV2(angle);
			}
			updateMesh();
		}

		void onDeath() override {
			queueDestroy();
		}

		AAnimaEnemy& moveTo(Math::Vector2 const& position, usize const time) {
			movement.reinterpolate(trans.position, position, time);
			return *this;
		}

		AAnimaEnemy& moveTo(Math::Vector2 const& position, usize const time, Math::Ease::Mode const& mode) {
			movement.setInterpolation(trans.position, position, time, mode);
			return *this;
		}

		AAnimaEnemy& accelerateTo(float const& newSpeed, usize const time) {
			acceleration.reinterpolate(speed, newSpeed, time);
			return *this;
		}

		AAnimaEnemy& accelerateTo(float const& newSpeed, usize const time, Math::Ease::Mode const& mode) {
			acceleration.setInterpolation(speed, newSpeed, time, mode);
			return *this;
		}

		AAnimaEnemy& turnTo(float const& newAngle, usize const time) {
			turning.reinterpolate(speed, newAngle, time);
			return *this;
		}

		AAnimaEnemy& turnTo(float const& newAngle, usize const time, Math::Ease::Mode const& mode) {
			turning.setInterpolation(speed, newAngle, time, mode);
			return *this;
		}

		float speed = 0;
		float angle = 0;
		
	protected:
		Math::Vector2 direction;

		void updateMesh() {
			mesh.trans.position		= trans.position;
			mesh.trans.rotation.z	= trans.rotation;
			mesh.trans.scale		= trans.scale;
		}

		bool execute(usize const name, Parameters const& params) override {
			switch (name) {
				case (ConstHasher::hash("move-to")):		solveMoveTo(params);		return true;
				case (ConstHasher::hash("accelerate-to")):	solveAccelerateTo(params);	return true;
				case (ConstHasher::hash("turn-to")):		solveTurnTo(params);		return true;
				case (ConstHasher::hash("scale")):			solveScale(params);			return true;
				case (ConstHasher::hash("radius")):			solveRadius(params);		return true;
				case (ConstHasher::hash("speed")):			solveSpeed(params);			return true;
				case (ConstHasher::hash("angle")):			solveAngle(params);			return true;
			}
			return false;
		}

		void solveMoveTo(Parameters const& params) {
			if (params.size() < 2) return;
			Math::Vector2 to;
			if (params[0].front() == '@') getTargetPosition(to, params[0]);
			else to = toVector<2>(params[0], trans.position);
			usize time;
			try {
				time = toUInt64(params[1]);
			} catch (...) {
				throw Error::InvalidValue(
					toString("Invalid value of [" , params[2], "] for number!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
			if (params.size() == 3)
				moveTo(to, time, toEaseMode(params[3]));
			else moveTo(to, time);
		}

		void solveAccelerateTo(Parameters const& params) {
			if (params.size() < 2) return;
			float to = toVector<1>(params[0], speed);
			usize time;
			try {
				time = toUInt64(params[1]);
			} catch (...) {
				throw Error::InvalidValue(
					toString("Invalid value of [" , params[2], "] for number!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
			if (params.size() == 3)
				accelerateTo(to, time, toEaseMode(params[3]));
			else accelerateTo(to, time);
		}

		void solveTurnTo(Parameters const& params) {
			if (params.size() < 2) return;
			float to;
			if (params[0].front() == '@') getAngleToTarget(to, params[0]);
			else to = toVector<1>(params[0], speed);
			usize time;
			try {
				time = toUInt64(params[1]);
			} catch (...) {
				throw Error::InvalidValue(
					toString("Invalid value of [" , params[2], "] for number!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
			if (params.size() == 3)
				turnTo(to, time, toEaseMode(params[3]));
			else turnTo(to, time);
		}

		void solveScale(Parameters const& params) {
			if (params.empty()) return;
			trans.scale = toVector<2>(params.front());
		}

		void solveRadius(Parameters const& params);

		void solveSpeed(Parameters const& params) {
			if (params.empty()) return;
			speed = toVector<1>(params.front());
		}

		void solveAngle(Parameters const& params) {
			if (params.empty()) return;
			if (params.front().front() == '@')	getAngleToTarget(angle, params.front());
			else								angle = toVector<1>(params.front());
		}

		void getTargetPosition(Math::Vector2& value, String const& param) {
			if (param.empty()) return;
			StringList const params = param.split(':');
			if (params.size() < 1) return;
			usize const type	= ConstHasher::hash(params[0]);
			String const name	= params.size() < 2 ? "" : params[1];
			Reference<AGameObject> target = solver.getTarget(type, name);
			Math::Vector2 result = 0;
			result = target->trans.position;
			if (params.size() > 2)
				result += toVector<2>(params[2], 0);
			value = result;
		}

		void getAngleToTarget(float& value, String const& param) {
			if (param.empty()) return;
			StringList const params = param.split(':');
			if (params.size() < 1) return;
			usize const type	= ConstHasher::hash(params[0]);
			String const name	= params.size() < 2 ? "" : params[1];
			Reference<AGameObject> target = solver.getTarget(type, name);
			float result = 0;
			result = trans.position.angleTo(target->trans.position);
			if (params.size() > 2)
				result += toVector<1>(params[2], 0);
			value = result;
		}

		Tween<Math::Vector2>	movement;
		Tween<float>			acceleration;
		Tween<float>			turning;

	private:
		
	};
}

#endif