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
		using CollisionMask = ColliderConfig::CollisionMask;
		ColliderConfig const hitbox = {
			CollisionLayer::ENEMY,
			CollisionLayer::PLAYER_ATTACK,
			CollisionTag::FOR_PLAYER_1
		};
		struct Collision {
			struct Player {
				CollisionMask const bullet	= CollisionLayer::PLAYER_BULLET;
				CollisionMask const laser	= CollisionLayer::PLAYER_LASER;
				CollisionMask const body	= CollisionLayer::PLAYER_COLLISION;
				CollisionMask const attack	= CollisionLayer::PLAYER_ATTACK;
			} const player = {};
			struct Tag {
				CollisionMask const player	= CollisionTag::FOR_PLAYER_1;
			} const tag = {};
		} const mask = {};
	};

	struct AEnemy: AGameObject, AUpdateable, IDamageable, Healthy {
		AEnemy(EnemyConfig const& cfg): AGameObject({cfg, cfg.hitbox}), mask(cfg.mask) {
		}

		constexpr static usize INVINCIBLE	= 1 << 0;
		constexpr static usize DEAD			= 1 << 1;

		usize flags = 0;

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

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (!isForThisPlayer(collider)) return;
			if (collider.affects.match(mask.player.attack).overlap())
				takeDamage(collider.data.reinterpret<AGameObject>(), collider.affects);
		}
		
		bool isForThisPlayer(Collider const& collider) const {
			return collider.tags.match(mask.tag.player).overlap();
		}

		AEnemy& takeDamage(Reference<AGameObject> const& object, CollisionMask const& collider) override {
			if (object) {
				if (collider.match(mask.player.bullet).overlap())
					takeDamage(object.as<Bullet>()->getDamage());
				else if (collider.match(mask.player.laser).overlap())
					takeDamage(object.as<Laser>()->getDamage());
			}
			return *this;
		}

		AEnemy& takeDamage(float const damage) override {
			if (areAnyFlagsSet(DEAD)) return;
			if (health > 0)
				loseHealth(damage);
			else {
				setFlags(DEAD, true);
				onDeath();
			}
			return *this;
		}

		virtual void onDeath() = 0;

		EnemyConfig::Collision const mask;

		AEnemy& setFlags(usize const mask, bool const state = true) {
			if (state)	flags |= mask;
			else		flags &= ~mask;
			return *this;
		}

		bool areAnyFlagsSet(usize const mask) const {
			return flags & mask;
		}

		bool areAllFlagsSet(usize const mask) const {
			return (flags & mask) == mask;
		}
	};
}

#endif