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
			Danmaku::Collision::Layer::ENEMY,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		};
		CollisionLayerConfig const hitboxLayer = {
			Danmaku::Collision::Mask::ENEMY,
			Danmaku::Collision::Mask::PLAYER_ATTACK
		};
		struct Collision {
			struct Player {
				CollisionMask const bullet	= Danmaku::Collision::Mask::PLAYER_BULLET;
				CollisionMask const laser	= Danmaku::Collision::Mask::PLAYER_LASER;
				CollisionMask const body	= Danmaku::Collision::Mask::PLAYER_COLLISION;
				CollisionMask const attack	= Danmaku::Collision::Mask::PLAYER_ATTACK;
			} const player = {};
			struct Tag {
				CollisionMask const player	= Danmaku::Collision::Tag::FOR_PLAYER_1;
			} const tag = {};
		} const mask = {};
	};

	struct AEnemy: AGameObject, AUpdateable, IDamageable, Healthy {
		AEnemy(EnemyConfig const& cfg): AGameObject({cfg, cfg.hitbox}), mask(cfg.mask) {
			collision()->getLayer().affects		= cfg.hitboxLayer.affects;
			collision()->getLayer().affectedBy	= cfg.hitboxLayer.affectedBy;
		}

		constexpr static usize INVINCIBLE	= 1 << 0;
		constexpr static usize DEAD			= 1 << 1;

		usize flags = 0;

		void onUpdate(float delta) override {
			if (!active) return;
			AGameObject::onUpdate(delta);
			if (paused()) return;
		}

		void onUpdate(float delta, Makai::App& app) override {
			if (!active) return;
			onUpdate(delta);
			if (paused()) return;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (!isForThisPlayer(collider)) return;
			if (collider.getLayer().affects.match(mask.player.attack).overlap())
				takeDamage(collider.data.mutate<>().as<AGameObject>(), collider.getLayer().affects);
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
			if (areAnyFlagsSet(DEAD)) return *this;
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