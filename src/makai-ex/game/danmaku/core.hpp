#ifndef MAKAILIB_EX_GAME_DANMAKU_CORE_H
#define MAKAILIB_EX_GAME_DANMAKU_CORE_H

#include <makai/makai.hpp>

#include "layers.hpp"
#include "../core/sprite.hpp"

/// @brief Danmaku-specific game extensions.
namespace Makai::Ex::Game::Danmaku {
	/// @brief Collision facility.
	namespace C2D = CTL::Ex::Collision::C2D;

	/// @brief Collision server.
	using CollisionServer = C2D::Server;

	/// @brief Interpolatable property.
	/// @tparam T Property type.
	template<Type::Ex::Tween::Tweenable T>
	struct Property {
		/// @brief Current value.
		T					value		= 0;
		/// @brief Whether to interpolate the property.
		bool				interpolate	= false;
		/// @brief Starting value.
		T					start		= 0;
		/// @brief End value.
		T					stop		= 0;
		/// @brief Interpolation speed.
		float				speed		= 0;
		/// @brief Interpolation function.
		Math::Ease::Mode	ease		= Math::Ease::linear;
		/// @brief Current interpolation factor.
		float				factor		= 0;

		/// @brief Updates the property, and returns its current value.
		/// @return Current value after processing.
		constexpr T next() {
			if (!(interpolate && speed != 0))
				return value;
			factor = Math::clamp<float>(factor, 0, 1);
			if (factor == 0)		value = start;
			else if (factor < 1)	value = Math::lerp<T>(start, stop, ease(factor));
			else					value = stop;
			factor += speed;
			return value;
		}

		/// @brief Reverses the property.
		/// @return Reference to self.
		constexpr Property& reverse() {
			CTL::swap(start, stop);
			factor = 1 - factor;
		}
	};
	/// @brief Pause state.
	struct PauseState {
		/// @brief Duration.
		llong	time	= -1;
		/// @brief Whether pause is enabled.
		bool	enabled	= false;
	};

	/// @brief Game area.
	struct GameArea {
		/// @brief Area center.
		Vector2 center;
		/// @brief Area size.
		Vector2 size;

		/// @brief Returns the lowest corner in the area.
		/// @return Lowest point in area.
		constexpr Vector2 min() const			{return center - size;						}
		/// @brief Returns the highest corner in the area.
		/// @return Highest point in area.
		constexpr Vector2 max() const			{return center + size;						}

		/// @brief Returns the game area's top left corner.
		/// @return Top left corner.
		constexpr Vector2 topLeft() const		{return center + size * Vector2(-1, +1);	}
		/// @brief Returns the game area's top right corner.
		/// @return Top right corner.
		constexpr Vector2 topRight() const		{return center + size * Vector2(+1, +1);	}
		/// @brief Returns the game area's bottom left corner.
		/// @return Bottom left corner.
		constexpr Vector2 bottomLeft() const	{return center + size * Vector2(-1, -1);	}
		/// @brief Returns the game area's bottom right corner.
		/// @return Bottom right corner.
		constexpr Vector2 bottomRight() const	{return center + size * Vector2(+1, -1);	}

		/// @brief Returns the game area as a collision shape.
		/// @return Game area as collision shape.
		constexpr C2D::Box asShape() const		{return C2D::Box(center, size);				}
		/// @brief Returns the game area as an AABB.
		/// @return Game area as AABB.
		constexpr C2D::AABB2D aabb() const		{return {min(), max()};						}
	};

	/// @brief Bounded object configuration.
	struct BoundedObjectConfig {
		/// @brief Game board.
		GameArea&			board;
		/// @brief Game playfield.
		GameArea&			playfield;
	};

	struct ColliderConfig {
		using CollisionMask = Collision::Mask::MaskType;
		uint64 const		layer		= 0;
		CollisionMask const	tags		= {};
	};

	struct CollisionLayerConfig {
		using CollisionMask = Collision::Mask::MaskType;
		CollisionMask const	affects		= {};
		CollisionMask const	affectedBy	= {};
	};

	struct GameObjectConfig: BoundedObjectConfig, ColliderConfig {};

	struct AGameObject: C2D::Collider<>::IData {
		using PromiseType			= Makai::Co::Promise<usize, true>;
		using Collider				= CollisionServer::Collider;
		using CollisionArea			= C2D::Area;
		using CollisionDirection	= C2D::Direction;
		using CollisionMask			= GameObjectConfig::CollisionMask;

		AGameObject(GameObjectConfig const& cfg):
			colli(cfg),
			board(cfg.board),
			playfield(cfg.playfield) {
				DEBUGLN("Building game object...");
				collider = CollisionServer::createCollider(cfg.layer);
				bindCollisionHandler(*this);
			}

		virtual ~AGameObject() {}

		PromiseType task;

		PauseState pause;

		Math::Transform2D trans;

		usize spawnTime		= 5;
		usize despawnTime	= 5;
		
		virtual AGameObject& spawn()	= 0;
		virtual AGameObject& despawn()	= 0;

		virtual void onUpdate(float) {
			if (!active) return;
			if (pause.enabled && pause.time > 0) {
				--pause.time;
				return;
			} else if (pause.enabled) {
				pause.time		= -1;
				pause.enabled	= false;
				onUnpause();
			}
			if (delay > 0) {
				--delay;
				return;
			}
			while (!delay && task)
				delay = task.next();
		}
		
		bool paused() const {
			if (pause.enabled)
				return pause.time > 0;
			return false;
		}

		virtual void onCollision(Collider const& collider, CollisionDirection const direction) = 0;

		ColliderConfig const colli;

	protected:
		GameArea&	board;
		GameArea&	playfield;

		virtual void onUnpause() {}

		void resetCollisionState() {
			collider->tags = colli.tags;
		}

		static PromiseType doNothing() {co_return 1;}

		bool active = false;

		Reference<Collider> collision() const {
			return collider.reference();
		}

	private:
		static void bindCollisionHandler(AGameObject& self) {
			self.collider->onCollision = (
				[&] (Collider const& collider, CollisionDirection const direction) -> void {
					self.onCollision(collider, direction);
				}
			);
			self.collider->data = &self;
		}

		Unique<Collider> collider;

		usize delay = 0;
	};

	struct ISpriteContainer {
		struct SpriteSetting {
			Vector2 frame;
			Vector2 sheetSize;
		} sprite;

		virtual ISpriteContainer& setSpriteRotation(float const angle)	= 0;
		virtual float getSpriteRotation() const							= 0;
		virtual ~ISpriteContainer() {}
	};

	struct ThreePatchContainer {
		struct PatchFrame {
			Vector2 head = Vector2(0, 0);
			Vector2 body = Vector2(1, 0);
			Vector2 tail = Vector2(2, 0);
		};

		struct PatchSetting {
			PatchFrame	frame		= {};
			Vector2		size		= 1;
			bool		vertical	= false;
		} patch;
	};

	struct AttackObject {
		Property<float>	velocity;
		Property<float>	rotation;
		Property<float>	damage;

		float getDamage() {
			return autoDecay ? damage.value : damage.next();
		}

		bool autoDecay = false;
	};

	struct Circular {
		Property<Vector2> radius;
	};

	struct Long {
		Property<float>	length;
	};

	struct Glowing {
		Property<float> glow;
		bool			glowOnSpawn = true;
	};

	struct Magnetizable {
		struct MagnetSetting {
			bool			enabled		= false;
			Handle<Vector2>	target		= nullptr;
			Property<float> strength	= {};
		} magnet;
	};

	struct IDamageable {
		virtual ~IDamageable() {}

		virtual IDamageable& takeDamage(
			Reference<AGameObject> const& object,
			Collision::Mask::MaskType const& collider
		) = 0;
		virtual IDamageable& takeDamage(float const damage) = 0;
	};

	struct IToggleable {
		enum State {
			TS_UNTOGGLED,
			TS_TOGGLING,
			TS_TOGGLED,
			TS_UNTOGGLING,
		};
		usize toggleTime	= 5;
		usize untoggleTime	= 5;
		virtual IToggleable& toggle(bool const state, bool const immediately = false) = 0;
		bool isToggled() {return toggleState == State::TS_TOGGLED;}
		virtual ~IToggleable() {}
	protected:
		State toggleState = State::TS_UNTOGGLED;
	};

	struct Weighted {
		Property<Vector2> gravity;
		Property<Vector2> terminalVelocity;
	};

	struct Dope {
		/// @brief `DOPE`: Destroy On Playfield Exit.
		bool dope = false;
	};

	struct RotatesSprite {
		bool rotateSprite = true;
	};

	struct Healthy {
		virtual ~Healthy() {}

		constexpr Healthy& setHealth(float const hp)	{health = hp < maxHealth ? hp : maxHealth; return *this;	}
		constexpr Healthy& gainHealth(float const hp)	{setHealth(hp + health); return *this;						}
		constexpr Healthy& loseHealth(float const hp)	{health -= hp; return *this;								}

		constexpr Healthy& setHealth(float const hp, float maxHP) {
			maxHealth = maxHP;
			return setHealth(hp);
		}

		constexpr float getHealth() const {return health;	}

	protected:
		float maxHealth	= 100;
		float health	= 100;
	};

	struct IKillable {
		virtual ~IKillable() {}

		virtual IKillable& die()	= 0;

		virtual void onDeath()		= 0;
	};
}

#endif