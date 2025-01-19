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
	struct PlayerConfig: BoundedObjectConfig {
		using CollisionMask = ColliderConfig::CollisionMask;
		ColliderConfig const hitbox = {
			{},
			CollisionLayer::ENEMY_MASK,
			CollisionTag::FOR_PLAYER_1
		};
		ColliderConfig const grazebox = {
			{},
			CollisionLayer::ENEMY_BULLET | CollisionLayer::ENEMY_LASER | CollisionLayer::ITEM,
			CollisionTag::FOR_PLAYER_1
		};
		ColliderConfig const itembox = {
			{},
			CollisionLayer::ITEM,
			CollisionTag::FOR_PLAYER_1
		};
		struct Collision {
			CollisionMask const item			= CollisionLayer::ITEM;
			struct Enemy {
				CollisionMask const bullet		= CollisionLayer::ENEMY_BULLET;
				CollisionMask const laser		= CollisionLayer::ENEMY_LASER;
				CollisionMask const body		= CollisionLayer::ENEMY_COLLISION;
				CollisionMask const attacker	= CollisionLayer::ENEMY_MASK;
			} const enemy = {};
			struct Tag {
				CollisionMask const player		= CollisionTag::FOR_PLAYER_1;
			} const tag = {};
		} const mask = {};
	};

	struct APlayer: Controllable, AGameObject, AUpdateable {
		struct Velocity {
			Vector2 focused		= 0;
			Vector2 unfocused	= 0;
		};

		APlayer(PlayerConfig const& cfg): AGameObject({cfg, cfg.hitbox}), mask(cfg.mask) {
			bindmap = Dictionary<String>({
				{"up",		"player/up"		},
				{"down",	"player/down"	},
				{"left",	"player/left"	},
				{"right",	"player/right"	},
				{"shot",	"player/shot"	},
				{"bomb",	"player/bomb"	},
				{"focus",	"player/focus"	}
			});
			bindGrazeHandler(*this);
			bindItemMagnetHandler(*this);
		}

		virtual ~APlayer() {
			Instance<Vector2>::detach(&trans.position);
		}

		constexpr static usize CAN_MOVE		= 1 << 0;
		constexpr static usize CAN_FOCUS	= 1 << 1;
		constexpr static usize CAN_SHOOT	= 1 << 2;
		constexpr static usize CAN_BOMB		= 1 << 3;
		constexpr static usize INVINCIBLE	= 1 << 3;

		usize flags = CAN_MOVE | CAN_FOCUS | CAN_SHOOT | CAN_BOMB;

		void onUpdate(float delta) override {
			if (!active) return;
			AGameObject::onUpdate(delta);
			if (paused()) return;
			friction.clamp(0, 1);
			pollInputs();
			doMovement(delta);
			updateTimers();
			if (action("bomb", true)	&& !bombTime && areAnyFlagsSet(CAN_BOMB))	onBomb();
			if (action("shot")			&& !shotTime && areAnyFlagsSet(CAN_SHOOT))	onShot();
		}

		void onUpdate(float delta, App& app) override {
			if (!active) return;
			onUpdate(delta);
			if (paused()) return;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (!isForThisPlayer(collider)) return;
			if (collider.affects.match(mask.enemy.attacker).overlap())
				getHurt(collider.data.reinterpret<AGameObject>());
		}

		virtual void onGrazeboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (!isForThisPlayer(collider)) return;
			if (collider.affects.match(
				mask.enemy.bullet
			|	mask.enemy.laser
			).overlap())
				if (auto bullet = collider.data.reinterpret<Bullet>())
					onGraze(bullet.as<AServerObject>());
			if (collider.affects.match(mask.enemy.laser).overlap())
				if (auto laser = collider.data.reinterpret<Laser>())
					onGraze(laser.as<AServerObject>());
			if (collider.affects.match(mask.item).overlap())
				if (auto item = collider.data.reinterpret<Item>()) {
					onItem(item);
					item->discard();
				}
		}

		virtual void onItemboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (!isForThisPlayer(collider)) return;
			if (collider.affects.match(mask.item).overlap())
				if (auto item = collider.data.reinterpret<Item>())
					onItemMagnet(item);
		}

		bool focused() const			{return isFocused;}
		Vector2 getDirection() const	{return direction;}

		APlayer& disableBomb(usize const frames) {
			bombTime = frames;
			return *this;
		}
		
		APlayer& disableShot(usize const frames) {
			shotTime = frames;
			return *this;
		}
		
		APlayer& makeInvincible(usize const frames) {
			invincibleTime = frames;
			return *this;
		}

		APlayer& setFlags(usize const mask, bool const state = true) {
			if (state)	flags |= mask;
			else		flags &= ~mask;
			return *this;
		}

		APlayer& getHurt(Reference<AGameObject> const& object) {
			if (invincibleTime || areAnyFlagsSet(INVINCIBLE)) return *this;
			onDamage(object);
			return *this;
		}

		bool areAnyFlagsSet(usize const mask) const {
			return flags & mask;
		}

		bool areAllFlagsSet(usize const mask) const {
			return (flags & mask) == mask;
		}

		bool isForThisPlayer(Collider const& collider) const {
			return collider.tags.match(mask.tag.player).overlap();
		}

		Vector2		friction	= 1;
		Velocity	velocity	= {};

		PlayerConfig::Collision const mask;

	protected:
		virtual void onItemMagnet(Reference<Item> const& item) {
			if (!item->magnet.enabled && item->magnet.target != &trans.position)
				item->magnet = {true, &trans.position, {1}};
		}
		virtual void onItem(Reference<Item> const& item)				= 0;
		virtual void onGraze(Reference<AServerObject> const& object)	= 0;
		virtual void onBomb()											= 0;
		virtual void onShot()											= 0;
		virtual void onDamage(Reference<AGameObject> const& object)		= 0;

		Reference<Collider> getGrazebox() {
			return grazebox.reference();
		}

		Reference<Collider> getItembox() {
			return grazebox.reference();
		}

	private:
		static void bindGrazeHandler(APlayer& self) {
			self.grazebox->onCollision = [&self = self] (Collider const& collider, CollisionDirection const direction) {
				self.onGrazeboxCollision(collider, direction);
			};
			self.grazebox->data = &self;
		}

		static void bindItemMagnetHandler(APlayer& self) {
			self.itembox->onCollision = [&self = self] (Collider const& collider, CollisionDirection const direction) {
				self.onItemboxCollision(collider, direction);
			};
			self.itembox->data = &self;
		}

		usize shotTime			= 0;
		usize bombTime			= 0;
		usize invincibleTime	= 0;

		Unique<Collider> grazebox	= CollisionServer::createCollider();
		Unique<Collider> itembox	= CollisionServer::createCollider();

		void pollInputs() {
			direction.y	= action("up") - action("down");
			direction.x	= action("right") - action("left");
			isFocused	= action("focus");
		}

		void doMovement(float const delta) {
			if (!areAnyFlagsSet(CAN_MOVE)) return;
			Vector2 const& vel = focused() ? velocity.focused : velocity.unfocused;
			if (friction.min() < 1) {
				speed = Math::lerp<Vector2>(speed, vel, friction);
				trans.position += direction * speed * delta;
			} else trans.position += direction * vel * delta;
		}

		void updateTimers() {
			if (bombTime > 0)		--bombTime;
			if (shotTime > 0)		--shotTime;
			if (invincibleTime > 0)	--invincibleTime;
		}

		Vector2 speed = 0;

		Vector2 direction;

		bool isFocused = 0;
	};
}

#endif