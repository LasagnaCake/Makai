#ifndef MAKAILIB_EX_GAME_DANMAKU_ENEMY_H
#define MAKAILIB_EX_GAME_DANMAKU_ENEMY_H

#include <makai/makai.hpp>

#include "core.hpp"
#include "bullet.hpp"
#include "laser.hpp"

#include "../core/controlable.hpp"
#include "../core/sprite.hpp"

/// @brief Danmaku-specific game extensions.
namespace Makai::Ex::Game::Danmaku {
	/// @brief Enemy configuration.
	struct EnemyConfig: BoundedObjectConfig {
		/// @brief Collision mask type.
		using CollisionMask = ColliderConfig::CollisionMask;
		/// @brief Hitbox configuration.
		ColliderConfig const hitbox = {
			Danmaku::Collision::Layer::ENEMY,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		};
		/// @brief Hitbox layer configuration.
		CollisionLayerConfig const hitboxLayer = {
			Danmaku::Collision::Mask::ENEMY,
			Danmaku::Collision::Mask::PLAYER_ATTACK
		};
		/// @brief Collision mask & tags.
		struct Collision {
			/// @brief Player masks.
			struct Player {
				CollisionMask const bullet	= Danmaku::Collision::Mask::PLAYER_BULLET;
				CollisionMask const laser	= Danmaku::Collision::Mask::PLAYER_LASER;
				CollisionMask const body	= Danmaku::Collision::Mask::PLAYER_COLLISION;
				CollisionMask const attack	= Danmaku::Collision::Mask::PLAYER_ATTACK;
			} const player = {};
			/// @brief Collision tags.
			struct Tag {
				CollisionMask const player	= Danmaku::Collision::Tag::FOR_PLAYER_1;
			} const tag = {};
		} const mask = {};
	};

	/// @brief Enemy abstract base.
	struct AEnemy: AGameObject, AUpdateable, IDamageable, IKillable, Healthy, Flaggable {
		/// @brief Enemy flags.
		struct Flags {
			/// @brief Invincible flag.
			constexpr static usize EF_INVINCIBLE	= 1 << 0;
			/// @brief Dead flag.
			constexpr static usize EF_DEAD			= 1 << 1;
			/// @brief Default starting flags.
			constexpr static usize DEFAULT			= 0;
		};

		/// @brief Health bar display.
		Graph::RadialBar healthBar;

		/// @brief Constructs the enemy.
		/// @param cfg Enemy configuration to use.
		AEnemy(EnemyConfig const& cfg): AGameObject({cfg, cfg.hitbox}), Flaggable{Flags::DEFAULT}, mask(cfg.mask) {
			collision()->getLayer().affects		= cfg.hitboxLayer.affects;
			collision()->getLayer().affectedBy	= cfg.hitboxLayer.affectedBy;
			collision()->canCollide = true;
			active = true;
		}

		/// @brief Called every execution cycle.
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

		/// @brief Called every execution cycle.
		void onUpdate(float delta, Makai::App& app) override {
			if (!active) return;
			onUpdate(delta);
			if (paused()) return;
		}

		/// @brief Called when a collision event with the enemy's hitbox happens.
		/// @param collider Collider colliding with the enemy's hitbox.
		/// @param direction Direction in which collision happens.
		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			//DEBUGLN("Collision event!\nFlags: ", collider.tags);
			if (isInvincible() || !isForThisPlayer(collider)) return;
			//DEBUGLN("Layer: ", collider.getLayer().affects);
			if (collider.getLayer().affects & mask.player.attack)
				takeDamage(collider.data.mutate<>().as<AGameObject>(), collider.getLayer().affects);
		}
		
		/// @brief Returns whether the collider is tagged for the player this object is associated with.
		/// @param collider Collider to check tag.
		/// @return Whether collider is tagged for this object's player.
		bool isForThisPlayer(Collider const& collider) const {
			return collider.tags & mask.tag.player;
		}

		/// @brief Recieves damage from a source.
		/// @param object Source object.
		/// @param collider Source object collision.
		/// @return Reference to self.
		AEnemy& takeDamage(Reference<AGameObject> const& object, CollisionMask const& collider) override {
			if (!isInvincible() && object) {
				if (collider & mask.player.bullet)
					takeDamage(object.as<Bullet>()->getDamage());
				else if (collider & mask.player.laser)
					takeDamage(object.as<Laser>()->getDamage());
				if (collider & (mask.player.bullet | mask.player.laser)) {
					//DEBUGLN("Owie! :'(");
					object.as<AServerObject>()->discard();
				}
			}
			return *this;
		}

		/// @brief Recieves damage from a source.
		/// @param damage Damage to take.
		/// @return Reference to self.
		AEnemy& takeDamage(float const damage) override {
			if (isInvincible() || areAnyFlagsSet(Flags::EF_DEAD)) return *this;
			if (health > 0)
				loseHealth(damage);
			else die();
			return *this;
		}

		/// @brief Kills the object.
		/// @return Reference to self.
		AEnemy& die() override {
			if (!isInvincible()) {
				setFlags(Flags::EF_DEAD, true);
				onDeath();
			}
			return *this;
		}

		/// @brief Collision mask associated with the enemy.
		EnemyConfig::Collision const mask;

		/// @brief Returns whether the enemy is currently invincible.
		/// @return Whether enemy is invincible.
		bool isInvincible() const {
			return areAnyFlagsSet(Flags::EF_INVINCIBLE);
		}
	};
}

#endif