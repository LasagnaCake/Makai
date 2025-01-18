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
	struct PlayerConfig: GameObjectConfig {};

	struct APlayer: Controllable, AGameObject, AUpdateable {
		struct Velocity {
			Vector2 focused		= 0;
			Vector2 unfocused	= 0;
		};

		APlayer(PlayerConfig const& cfg): AGameObject(cfg) {
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

		usize actions = CAN_MOVE | CAN_FOCUS | CAN_SHOOT | CAN_BOMB;

		void onUpdate(float delta) override {
			AGameObject::onUpdate(delta);
			if (!active || paused()) return;
			pollInputs();
			friction.clamp(0, 1);
			Vector2 const& vel = focused() ? velocity.focused : velocity.unfocused;
			if (friction < 1) {
				speed = Math::lerp<Vector2>(speed, vel, friction);
				trans.position += direction * speed * delta;
			} else trans.position += direction * vel * delta;
//			trans.position.clamp(playfield.topLeft(), playfield.bottomRight());
		}

		void onUpdate(float delta, App& app) override {
			onUpdate(delta);
			if (!active || paused()) return;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) {
			if (collider.affects.match(CollisionLayer::ENEMY_MASK).overlap())
				pichun();
		}

		virtual void onGrazeboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (collider.affects.match(CollisionLayer::ENEMY_BULLET).overlap())
				if (auto bullet = collider.data.reinterpret<Bullet>()) {
					if (!bullet->grazed) {
						bullet->grazed = true;
					}
				}
		}

		virtual void onItemboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (collider.affects.match(CollisionLayer::ITEM).overlap())
				if (auto item = collider.data.reinterpret<Item>()) {
					item->magnet = {
						true,
						&trans.position,
						itemMagnetStrength
					};
				}
		}

		virtual void onGrazeboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (collider.affects.match(CollisionLayer::ITEM).overlap())
				if (auto item = collider.data.reinterpret<Item>()) {
					onItem(item);
					item->discard();
				}
		}

		virtual void onItem(Reference<Item> const& item)				= 0;
		virtual void onGraze(Reference<AServerObject> const& object)	= 0;
		virtual void onBomb()											= 0;
		virtual void onShot()											= 0;

		Property<float>	itemMagnetStrength = {8};

		Vector2 friction	= 0;
		Velocity velocity	= {};

		bool focused() const			{return isFocused;}
		Vector2 getDirection() const	{return direction;}

	protected:
		void pichun() {

		}

		void pollInputs() {
			direction.y	= action("up") - action("down");
			direction.x	= action("right") - action("left");
			isFocused	= action("focus");
		}

	private:
		Vector2 speed = 0;

		Vector2 direction;

		bool isFocused = 0;
	};
}

#endif