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
		ColliderConfig const colli = {
			{},
			CollisionLayer::ENEMY_MASK,
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

		APlayer(PlayerConfig const& cfg): AGameObject({cfg, cfg.colli}), colli(cfg.mask) {
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

		virtual ~APlayer() {
			Instance<Vector2>::detach(&trans.position);
		}

		constexpr static usize CAN_MOVE		= 1 << 0;
		constexpr static usize CAN_FOCUS	= 1 << 1;
		constexpr static usize CAN_SHOOT	= 1 << 2;
		constexpr static usize CAN_BOMB		= 1 << 3;

		usize flags = CAN_MOVE | CAN_FOCUS | CAN_SHOOT | CAN_BOMB;

		void onUpdate(float delta) override {
			AGameObject::onUpdate(delta);
			if (!active || paused()) return;
			pollInputs();
			friction.clamp(0, 1);
			doMovement(delta);
		}

		void onUpdate(float delta, App& app) override {
			onUpdate(delta);
			if (!active || paused()) return;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) {
			if (
				collider.affects.match(colli.enemy.attacker).overlap()
			&&	collider.tags.match(colli.tag.player).overlap()
			)
				pichun(collider.data.reinterpret<AGameObject>());
		}

		virtual void onGrazeboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (
				collider.affects.match(colli.enemy.bullet).overlap()
			&&	collider.tags.match(colli.tag.player).overlap()
			)
				if (auto bullet = collider.data.reinterpret<Bullet>()) {
					if (!bullet->grazed) {
						bullet->grazed = true;
					}
				}
		}

		virtual void onItemboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (
				collider.affects.match(colli.item).overlap()
			&&	collider.tags.match(colli.tag.player).overlap()
			)
				if (auto item = collider.data.reinterpret<Item>())
					onItemMagnet(item);
		}

		virtual void onGrazeboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (
				collider.affects.match(colli.item).overlap()
			&&	collider.tags.match(colli.tag.player).overlap()
			)
				if (auto item = collider.data.reinterpret<Item>()) {
					onItem(item);
					item->discard();
				}
		}

		virtual void onItemMagnet(Reference<Item> const& item)			{item->magnet = {true, &trans.position, {1}};}
		virtual void onItem(Reference<Item> const& item)				= 0;
		virtual void onGraze(Reference<AServerObject> const& object)	= 0;
		virtual void onBomb()											= 0;
		virtual void onShot()											= 0;
		virtual void onDamage(Reference<AGameObject> const& object)		= 0;

		Vector2		friction	= 0;
		Velocity	velocity	= {};

		bool focused() const			{return isFocused;}
		Vector2 getDirection() const	{return direction;}

		usize bombTime = 60;
		usize shotTime = 5;

		PlayerConfig::Collision const colli;

	protected:
		void pichun(Reference<AGameObject> const& object) {
			onDamage(object);
		}

	private:
		void pollInputs() {
			direction.y	= action("up") - action("down");
			direction.x	= action("right") - action("left");
			isFocused	= action("focus");
		}

		void doMovement(float const delta) {
			if (!(flags & CAN_MOVE)) return;
			Vector2 const& vel = focused() ? velocity.focused : velocity.unfocused;
			if (friction.min() < 1) {
				speed = Math::lerp<Vector2>(speed, vel, friction);
				trans.position += direction * speed * delta;
			} else trans.position += direction * vel * delta;
		}

		Vector2 speed = 0;

		Vector2 direction;

		bool isFocused = 0;
	};
}

#endif