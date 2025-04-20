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
	/// @brief Player configuration.
	struct PlayerConfig: BoundedObjectConfig {
		/// @brief Collision mask type.
		using CollisionMask = ColliderConfig::CollisionMask;
		/// @brief Hitbox settings.
		ColliderConfig const hitbox = {
			Danmaku::Collision::Layer::PLAYER,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		};
		/// @brief Hitbox layer settings.
		CollisionLayerConfig const hitboxLayer = {
			Danmaku::Collision::Mask::PLAYER,
			Danmaku::Collision::Mask::ENEMY_MASK
		};
		/// @brief Grazebox settings.
		ColliderConfig const grazebox = {
			Danmaku::Collision::Layer::PLAYER_GRAZEBOX,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		};
		/// @brief Grazebox layer settings.
		CollisionLayerConfig const grazeboxLayer = {
			{},
			Danmaku::Collision::Mask::ENEMY_BULLET | Danmaku::Collision::Mask::ENEMY_LASER | Danmaku::Collision::Mask::ITEM
		};
		/// @brief Itembox settings.
		ColliderConfig const itembox = {
			Danmaku::Collision::Layer::PLAYER_ITEMBOX,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		};
		/// @brief Itembox layer settings.
		CollisionLayerConfig const itemboxLayer = {
			{},
			Danmaku::Collision::Mask::ITEM
		};
		/// @brief Collision mask & tags.
		struct Collision {
			/// @brief Item mask.
			CollisionMask const item		= Danmaku::Collision::Mask::ITEM;
			/// @brief Enemy masks.
			struct Enemy {
				CollisionMask const bullet	= Danmaku::Collision::Mask::ENEMY_BULLET;
				CollisionMask const laser	= Danmaku::Collision::Mask::ENEMY_LASER;
				CollisionMask const body	= Danmaku::Collision::Mask::ENEMY_COLLISION;
				CollisionMask const attack	= Danmaku::Collision::Mask::ENEMY_ATTACK;
			} const enemy = {};
			/// @brief Collision tags.
			struct Tag {
				CollisionMask const player	= Danmaku::Collision::Tag::FOR_PLAYER_1;
			} const tag = {};
		} const mask = {};
	};

	/// @brief Player abstract base.
	struct APlayer: Controllable, AGameObject, AUpdateable, IDamageable, Flaggable {
		/// @brief Player flags.
		struct Flags {
			/// @brief Can move flag.
			constexpr static usize PF_CAN_MOVE		= 1 << 0;
			/// @brief Can focus flag.
			constexpr static usize PF_CAN_FOCUS		= 1 << 1;
			/// @brief Can unfocus flag.
			constexpr static usize PF_CAN_UNFOCUS	= 1 << 2;
			/// @brief Can shoot flag.
			constexpr static usize PF_CAN_SHOOT		= 1 << 3;
			/// @brief Can bomb flag.
			constexpr static usize PF_CAN_BOMB		= 1 << 4;
			/// @brief Invincible flag.
			constexpr static usize PF_INVINCIBLE	= 1 << 5;
			/// @brief Default starting flags.
			constexpr static usize DEFAULT			=
				Flags::PF_CAN_MOVE
			|	Flags::PF_CAN_FOCUS
			|	Flags::PF_CAN_UNFOCUS
			|	Flags::PF_CAN_SHOOT
			|	Flags::PF_CAN_BOMB
			;
		};

		/// @brief Player velocity.
		struct Velocity {
			/// @brief Unfocused velocity.
			Vector2 unfocused	= 0;
			/// @brief Focused velocity.
			Vector2 focusing		= 0;
		};

		/// @brief Constructs the player.
		/// @param cfg Player configuration to use.
		APlayer(PlayerConfig const& cfg): AGameObject({cfg, cfg.hitbox}), Flaggable{Flags::DEFAULT}, mask(cfg.mask) {
			DEBUGLN("Building player...");
			DEBUGLN("Graze: ", Collision::Layer::asName(cfg.grazebox.layer));
			DEBUGLN("Item: ", Collision::Layer::asName(cfg.itembox.layer));
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

		/// @brief Destructor.
		virtual ~APlayer() {
			DEBUGLN("Demagnetizing player...");
			Instance<Vector2>::detach(&trans.position);
			DEBUGLN("Player demagnetized!");
		}

		/// @brief Called every execution cycle.
		void onUpdate(float delta) override {
			if (!active) return;
			AGameObject::onUpdate(delta);
			if (paused()) return;
			friction.clamp(0, 1);
			pollInputs();
			doMovement(delta);
			updateTimers();
			if (action("bomb", true) && canBomb())	onBomb();
			if (action("shot") && canShoot())		onShot();
		}

		/// @brief Called every execution cycle.
		void onUpdate(float delta, Makai::App& app) override {
			if (!active) return;
			onUpdate(delta);
			if (paused()) return;
		}
		
		/// @brief Called when a collision event with the player's hitbox happens.
		/// @param collider Collider colliding with the player's hitbox.
		/// @param direction Direction in which collision happens.
		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (!isForThisPlayer(collider)) return;
			if (collider.getLayer().affects & mask.enemy.attack && !isInvincible())
				takeDamage(collider.data.mutate<>().as<AGameObject>(), collider.getLayer().affects);
		}

		/// @brief Called when a collision event with the player's grazebox happens.
		/// @param collider Collider colliding with the player's grazebox.
		/// @param direction Direction in which collision happens.
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

		/// @brief Called when a collision event with the player's itembox happens.
		/// @param collider Collider colliding with the player's itembox.
		/// @param direction Direction in which collision happens.
		virtual void onItemboxCollision(Collider const& collider, CollisionDirection const direction) {
			if (!isForThisPlayer(collider)) return;
			if (collider.getLayer().affects & mask.item)
				if (auto item = collider.data.mutate<>().as<Item>())
					onItemMagnet(item);
		}

		/// @brief Returns whether the player is focusing.
		/// @return Whether player is focusing.
		bool focusing() const {
			if (areAllFlagsSet(Flags::PF_CAN_FOCUS | Flags::PF_CAN_UNFOCUS)) return isFocused;
			if (areAnyFlagsSet(Flags::PF_CAN_FOCUS)) return true;
			return false;
		}

		/// @brief Returns which direction the player is moving towards.
		Vector2 getDirection() const	{return direction;}

		/// @brief Disables bombing for a set period of time.
		/// @param frames Frames to disable bombing for.
		/// @return Reference to self.
		APlayer& disableBomb(usize const frames) {
			bombTime = frames;
			return *this;
		}
		
		/// @brief Disables shooting for a set period of time.
		/// @param frames Frames to disable shooting for.
		/// @return Reference to self.
		APlayer& disableShot(usize const frames) {
			shotTime = frames;
			return *this;
		}
		
		/// @brief Makes the player invincible for a set period of time.
		/// @param frames Frames to be invincible for.
		/// @return Reference to self.
		APlayer& makeInvincible(usize const frames) {
			invincibleTime = frames;
			return *this;
		}

		/// @brief Returns the remaining invincibility frames.
		/// @return Remaining invincibility frames.
		usize getRemainingIFrames() const {return invincibleTime;}

		/// @brief Returns whether the collider is tagged for the player this object is associated with.
		/// @param collider Collider to check tag.
		/// @return Whether collider is tagged for this object's player.
		bool isForThisPlayer(Collider const& collider) const {
			return collider.tags & mask.tag.player;
		}

		/// @brief Movement friction.
		Vector2		friction	= 1;
		/// @brief Movement velocity.
		Velocity	velocity	= {};

		/// @brief Mask associated with this player.
		PlayerConfig::Collision const mask;

		/// @brief Returns whether the player is invincible.
		/// @return Whether player is invincible.
		bool isInvincible() const {
			return invincibleTime || areAnyFlagsSet(Flags::PF_INVINCIBLE);
		}

		/// @brief Returns whether the player can bomb.
		/// @return Whether player can bomb.
		bool canBomb() const {
			return (!bombTime) && areAnyFlagsSet(Flags::PF_CAN_BOMB);
		}

		/// @brief Returns whether the player can shoot.
		/// @return Whether player can shoot.
		bool canShoot() const {
			return (!shotTime) && areAnyFlagsSet(Flags::PF_CAN_SHOOT);
		}

	protected:
		/// @brief Gets called when an item is requested to be magnetized.
		/// @param item Item to magnetize.
		virtual void onItemMagnet(Reference<Item> const& item) {
			if (!item->magnet.enabled)
				item->magnet = {
					true,
					&trans.position, {
						.interpolate = true,
						.start = 5,
						.stop = 60,
						.speed = 0.05
					}
				};
		}
		/// @brief Gets called when an item is collected. Must be implemented.
		virtual void onItem(Reference<Item> const& item)				= 0;
		/// @brief Gets called when an enemy attack is grazed. Must be implemented.
		virtual void onGraze(Reference<AServerObject> const& object)	= 0;
		/// @brief Gets called when the player bombs. Must be implemented.
		virtual void onBomb()											= 0;
		/// @brief Gets called when the player shoots. Must be implemented.
		virtual void onShot()											= 0;

		/// @brief Returns a refeference to the player's grazebox.
		/// @return Reference to grazebox.
		Reference<Collider> getGrazebox() {
			return grazebox.reference();
		}

		/// @brief Returns a refeference to the player's itembox.
		/// @return Reference to itembox.
		Reference<Collider> getItembox() {
			return itembox.reference();
		}

	private:
		static void bindGrazeHandler(APlayer& self) {
			self.grazebox->onCollision = (
				[&] (Collider const& collider, CollisionDirection const direction) -> void {
					self.onGrazeboxCollision(collider, direction);
				}
			);
			self.grazebox->data = &self;
		}

		static void bindItemMagnetHandler(APlayer& self) {
			self.itembox->onCollision = (
				[&] (Collider const& collider, CollisionDirection const direction) -> void {
					self.onItemboxCollision(collider, direction);
				}
			);
			self.itembox->data = &self;
		}

		/// @brief Remaining time without shooting.
		usize shotTime			= 0;
		/// @brief Remaining time without bombing.
		usize bombTime			= 0;
		/// @brief Remaining invincible time.
		usize invincibleTime	= 0;

		/// @brief Grazebox.
		Unique<Collider> grazebox;
		/// @brief Itembox.
		Unique<Collider> itembox;

		void pollInputs() {
			direction.y	= action("up") - action("down");
			direction.x	= action("right") - action("left");
			isFocused	= action("focus");
		}

		void doMovement(float const delta) {
			if (!areAnyFlagsSet(Flags::PF_CAN_MOVE)) return;
			Vector2 const& vel = focusing() ? velocity.focusing : velocity.unfocused;
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

		/// @brief Calculated speed.
		Vector2 speed = 0;

		/// @brief Calculated direction.
		Vector2 direction;

		/// @brief Whether the shift key is pressed.
		bool isFocused = false;
	};
}

#endif