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
			Danmaku::Collision::Layer::PLAYER,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		};
		CollisionLayerConfig const hitboxLayer = {
			Danmaku::Collision::Mask::PLAYER,
			Danmaku::Collision::Mask::ENEMY_MASK
		};
		ColliderConfig const grazebox = {
			Danmaku::Collision::Layer::PLAYER_GRAZEBOX,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		};
		CollisionLayerConfig const grazeboxLayer = {
			{},
			Danmaku::Collision::Mask::ENEMY_BULLET | Danmaku::Collision::Mask::ENEMY_LASER | Danmaku::Collision::Mask::ITEM
		};
		ColliderConfig const itembox = {
			Danmaku::Collision::Layer::PLAYER_ITEMBOX,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		};
		CollisionLayerConfig const itemboxLayer = {
			{},
			Danmaku::Collision::Mask::ITEM
		};
		struct Collision {
			CollisionMask const item		= Danmaku::Collision::Mask::ITEM;
			struct Enemy {
				CollisionMask const bullet	= Danmaku::Collision::Mask::ENEMY_BULLET;
				CollisionMask const laser	= Danmaku::Collision::Mask::ENEMY_LASER;
				CollisionMask const body	= Danmaku::Collision::Mask::ENEMY_COLLISION;
				CollisionMask const attack	= Danmaku::Collision::Mask::ENEMY_ATTACK;
			} const enemy = {};
			struct Tag {
				CollisionMask const player	= Danmaku::Collision::Tag::FOR_PLAYER_1;
			} const tag = {};
		} const mask = {};
	};

	struct APlayer: Controllable, AGameObject, AUpdateable, IDamageable {
		struct Velocity {
			Vector2 unfocused	= 0;
			Vector2 focused		= 0;
		};

		APlayer(PlayerConfig const& cfg): AGameObject({cfg, cfg.hitbox}), mask(cfg.mask) {
			DEBUGLN("Building player...");
			grazebox	= CollisionServer::createCollider(cfg.grazebox.layer);
			itembox		= CollisionServer::createCollider(cfg.itembox.layer);
			collision()->getLayer().affects		= cfg.hitboxLayer.affects;
			collision()->getLayer().affectedBy	= cfg.hitboxLayer.affectedBy;
			grazebox->getLayer().affects		= cfg.grazeboxLayer.affects;
			grazebox->getLayer().affectedBy		= cfg.grazeboxLayer.affectedBy;
			itembox->getLayer().affects			= cfg.itemboxLayer.affects;
			itembox->getLayer().affectedBy		= cfg.itemboxLayer.affectedBy;
			bindmap		= Dictionary<String>({
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
			active = true;
		}

		virtual ~APlayer() {
			DEBUGLN("Demagnetizing player...");
			Instance<Vector2>::detach(&trans.position);
			DEBUGLN("Player demagnetized!");
		}

		constexpr static usize CAN_MOVE		= 1 << 0;
		constexpr static usize CAN_FOCUS	= 1 << 1;
		constexpr static usize CAN_SHOOT	= 1 << 2;
		constexpr static usize CAN_BOMB		= 1 << 3;
		constexpr static usize INVINCIBLE	= 1 << 4;

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

		void onUpdate(float delta, Makai::App& app) override {
			if (!active) return;
			onUpdate(delta);
			if (paused()) return;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (!isForThisPlayer(collider)) return;
			if (collider.getLayer().affects & mask.enemy.attack)
				takeDamage(collider.data.mutate<>().as<AGameObject>(), collider.getLayer().affects);
		}

		virtual void onGrazeboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (!isForThisPlayer(collider)) return;
			if (collider.getLayer().affects & (mask.enemy.bullet | mask.enemy.laser))
				if (auto object = collider.data.mutate<>().as<AServerObject>())
					onGraze(object);
			if (collider.getLayer().affects & mask.item)
				if (auto item = collider.data.mutate<>().as<Item>()) {
					onItem(item);
					item->discard(true);
				}
		}

		virtual void onItemboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (!isForThisPlayer(collider)) return;
			if (collider.getLayer().affects & mask.item)
				if (auto item = collider.data.mutate<>().as<Item>())
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

		usize getRemainingIFrames() const {return invincibleTime;}

		APlayer& setFlags(usize const mask, bool const state = true) {
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

		bool isForThisPlayer(Collider const& collider) const {
			return collider.tags & mask.tag.player;
		}

		Vector2		friction	= 1;
		Velocity	velocity	= {};

		PlayerConfig::Collision const mask;

		bool isInvincible() const {
			return invincibleTime || areAnyFlagsSet(INVINCIBLE);
		}

	protected:
		virtual void onItemMagnet(Reference<Item> const& item) {
			if (!item->magnet.enabled && item->magnet.target != &trans.position)
				item->magnet = {true, &trans.position, {1}};
		}
		virtual void onItem(Reference<Item> const& item)				= 0;
		virtual void onGraze(Reference<AServerObject> const& object)	= 0;
		virtual void onBomb()											= 0;
		virtual void onShot()											= 0;

		Reference<Collider> getGrazebox() {
			return grazebox.reference();
		}

		Reference<Collider> getItembox() {
			return grazebox.reference();
		}

	private:
		static void bindGrazeHandler(APlayer& self) {
			self.grazebox->onCollision = (
				[&self = self] (Collider const& collider, CollisionDirection const direction) -> void {
					self.onGrazeboxCollision(collider, direction);
				}
			);
			self.grazebox->data = &self;
		}

		static void bindItemMagnetHandler(APlayer& self) {
			self.itembox->onCollision = (
				[&self = self] (Collider const& collider, CollisionDirection const direction) -> void {
					self.onItemboxCollision(collider, direction);
				}
			);
			self.itembox->data = &self;
		}

		usize shotTime			= 0;
		usize bombTime			= 0;
		usize invincibleTime	= 0;

		Unique<Collider> grazebox;
		Unique<Collider> itembox;

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