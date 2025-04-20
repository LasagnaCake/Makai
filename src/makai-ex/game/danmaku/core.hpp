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

	/// @brief Collider configuration.
	struct ColliderConfig {
		using CollisionMask = Collision::Mask::MaskType;
		/// @brief Layer the collider resides in.
		uint64 const		layer		= 0;
		/// @brief Collider tags.
		CollisionMask const	tags		= {};
	};

	/// @brief Collision layer configuration.
	struct CollisionLayerConfig {
		/// @brief Collision mask type.
		using CollisionMask = Collision::Mask::MaskType;
		/// @brief Which layers this layer can collide with.
		CollisionMask const	affects		= {};
		/// @brief Which layers can collide with this layer.
		CollisionMask const	affectedBy	= {};
	};

	/// @brief Danmaku game object configuration.
	struct GameObjectConfig: BoundedObjectConfig, ColliderConfig {};

	/// @brief Basic danmaku game object abstract base.
	struct AGameObject: C2D::Collider<>::IData {
		/// @brief Coroutine promise type.
		using PromiseType			= Makai::Co::Promise<usize, true>;
		/// @brief Collider type.
		using Collider				= CollisionServer::Collider;
		/// @brief Collision area type.
		using CollisionArea			= C2D::Area;
		/// @brief Collision direction type.
		using CollisionDirection	= C2D::Direction;
		/// @brief Collision mask type.
		using CollisionMask			= GameObjectConfig::CollisionMask;

		/// @brief Constructs the object.
		/// @param cfg Game object configuration.
		AGameObject(GameObjectConfig const& cfg):
			colli(cfg),
			board(cfg.board),
			playfield(cfg.playfield) {
				DEBUGLN("Building game object...");
				collider = CollisionServer::createCollider(cfg.layer);
				bindCollisionHandler(*this);
			}

		/// @brief Destructor.
		virtual ~AGameObject() {}

		/// @brief Coroutine task associated with the object.
		PromiseType task;

		/// @brief Pause state.
		PauseState pause;

		/// @brief 2D Transform.
		Math::Transform2D trans;

		/// @brief Spawn time.
		usize spawnTime		= 5;
		/// @brief Despawn time.
		usize despawnTime	= 5;
		
		/// @brief Spawns the object. Must be implemented. 
		virtual AGameObject& spawn()	= 0;
		/// @brief Despawns the object. Must be implemented.
		virtual AGameObject& despawn()	= 0;

		/// @brief Called every update cycle.
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
		
		/// @brief Returns whether the object is currently paused.
		/// @return Whether object is currently paused.
		bool paused() const {
			if (pause.enabled)
				return pause.time > 0;
			return false;
		}

		/// @brief Called when a collision event with the object's hitbox happens. Must be implemented.
		virtual void onCollision(Collider const& collider, CollisionDirection const direction) = 0;

		/// @brief Collider configuration.
		ColliderConfig const colli;

	protected:
		/// @brief Game board.
		GameArea&	board;
		/// @brief Game playfield.
		GameArea&	playfield;

		/// @brief Gets called when the object's timed pause is finished. Does not get called when pause is stopped early.
		virtual void onUnpause() {}

		/// @brief Resets the collider's tags to their original values.
		void resetCollisionTags() {
			collider->tags = colli.tags;
		}

		/// @brief Coroutine task that does nothing.
		/// @return Promise to coroutine. 
		static PromiseType doNothing() {co_return 1;}

		/// @brief Whether the object is currently active.
		bool active = false;

		/// @brief Returns a reference to the object's collider.
		/// @return Reference to collider.
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

		/// @brief Collider associated with the object.
		Unique<Collider> collider;

		/// @brief Time to wait until coroutine is resumed.
		usize delay = 0;
	};

	/// @brief Sprite container interface.
	struct ISpriteContainer {
		/// @brief Sprite settings.
		struct SpriteSetting {
			/// @brief Sprite frame.
			Vector2 frame;
			/// @brief Sprite sheet size.
			Vector2 sheetSize;
		} sprite;

		/// @brief Sets the sprite's rotation.
		/// @param angle Rotation angle.
		/// @return Reference to self.
		virtual ISpriteContainer& setSpriteRotation(float const angle)	= 0;
		/// @brief Returns the sprite's current rotation.
		/// @return Current sprite rotation.
		virtual float getSpriteRotation() const							= 0;
		/// @brief Destructor.
		virtual ~ISpriteContainer() {}
	};

	/// @brief Three patch shape container component.
	struct ThreePatchContainer {
		/// @brief Three-patch frame settings.
		struct PatchFrame {
			/// @brief Head frame.
			Vector2 head = Vector2(0, 0);
			/// @brief Body frame.
			Vector2 body = Vector2(1, 0);
			/// @brief Tail frame.
			Vector2 tail = Vector2(2, 0);
		};

		/// @brief Three-patch shape settings.
		struct PatchSetting {
			/// @brief Frame settings.
			PatchFrame	frame		= {};
			/// @brief Sheet size.
			Vector2		size		= 1;
			/// @brief Whether sprite sheet is vertical.
			bool		vertical	= false;
		} patch;
	};

	/// @brief Attack object component.
	struct AttackObject {
		/// @brief Velocity.
		Property<float>	velocity;
		/// @brief Rotation.
		Property<float>	rotation;
		/// @brief Damage.
		Property<float>	damage;

		/// @brief Returns the current damage.
		/// @return Current damage.
		float getDamage() {
			return autoDecay ? damage.value : damage.next();
		}

		/// @brief Whether damage decays automatically, or when `getDamage` is called.
		bool autoDecay = false;
	};

	/// @brief Circular object component.
	struct Circular {
		/// @brief Radius.
		Property<Vector2> radius;
	};

	/// @brief Long object component.
	struct Long {
		/// @brief Length.
		Property<float>	length;
	};

	/// @brief Glowing object component.
	struct Glowing {
		/// @brief Glow factor.
		Property<float> glow;
		/// @brief Whether to glow when spawning.
		bool			glowOnSpawn = true;
	};

	/// @brief Magnetizable object component.
	struct Magnetizable {
		/// @brief Magnet settings.
		struct MagnetSetting {
			/// @brief Whether object is currently magnetized.
			bool			enabled		= false;
			/// @brief Magnet target position.
			Handle<Vector2>	target		= nullptr;
			/// @brief Magnet strength.
			Property<float> strength	= {};
		} magnet;
	};

	/// @brief Damageable object interface.
	struct IDamageable {
		/// @brief Destructor.
		virtual ~IDamageable() {}

		/// @brief Recieves damage from a source. Must be implemented.
		virtual IDamageable& takeDamage(
			Reference<AGameObject> const& object,
			Collision::Mask::MaskType const& collider
		) = 0;
		/// @brief Recieves damage from a source. Must be implemented.
		virtual IDamageable& takeDamage(float const damage) = 0;
	};

	/// @brief Toggleable object interface.
	struct IToggleable {
		/// @brief Toggle state.
		enum State {
			TS_UNTOGGLED,
			TS_TOGGLING,
			TS_TOGGLED,
			TS_UNTOGGLING,
		};
		/// @brief Toggle time.
		usize toggleTime	= 5;
		/// @brief Untoggle time.
		usize untoggleTime	= 5;
		/// @brief Sets the object's toggle state. Must be implemented.
		virtual IToggleable& toggle(bool const state, bool const immediately = false) = 0;
		/// @brief Returns whether the object is currently toggled.
		/// @return Whether object is currently toggled.
		bool isToggled() {return toggleState == State::TS_TOGGLED;}
		/// @brief Destructor.
		virtual ~IToggleable() {}
	protected:
		/// @brief Current toggle state.	
		State toggleState = State::TS_UNTOGGLED;
	};

	/// @brief Weighted object component.
	struct Weighted {
		/// @brief Gravity.
		Property<Vector2> gravity;
		/// @brief Terminal velocity.
		Property<Vector2> terminalVelocity;
	};

	/// @brief DOPE (Destroyable On Playfield Exit) object component.
	struct Dope {
		/// @brief `DOPE`: Destroy On Playfield Exit.
		bool dope = false;
	};

	/// @brief Rotating sprite object component.
	struct RotatesSprite {
		/// @brief Whether to rotate sprite to match transform rotation.
		bool rotateSprite = true;
	};

	/// @brief Healthy object component.
	struct Healthy {
		/// @brief Destructor.
		~Healthy() {}

		/// @brief Sets the current health.
		/// @param hp Health to set.
		/// @return Reference to self.
		constexpr Healthy& setHealth(float const hp)	{health = hp < maxHealth ? hp : maxHealth; return *this;	}
		/// @brief Adds health to the current health.
		/// @param hp Health to add.
		/// @return Reference to self.
		constexpr Healthy& gainHealth(float const hp)	{setHealth(hp + health); return *this;						}
		/// @brief Removes health from the current health.
		/// @param hp Health to remove.
		/// @return Reference to self.
		constexpr Healthy& loseHealth(float const hp)	{health -= hp; return *this;								}

		/// @brief Sets the current health and max health.
		/// @param hp Health to set.
		/// @param maxHP Max health to set.
		/// @return Reference to self.
		constexpr Healthy& setHealth(float const hp, float maxHP) {
			maxHealth = maxHP;
			return setHealth(hp);
		}

		/// @brief Returns the current health.
		/// @return Current health.
		constexpr float getHealth() const {return health;	}

	protected:
		/// @brief Max health.
		float maxHealth	= 100;
		/// @brief Current health.
		float health	= 100;
	};

	/// @brief Killable object interface.
	struct IKillable {
		/// @brief Destructor.
		virtual ~IKillable() {}

		/// @brief Kills the object. Must be implemented.
		virtual IKillable& die()	= 0;

		/// @brief Called when the object dies. Must be implemented.
		virtual void onDeath()		= 0;
	};

	/// @brief Flaggable object component.
	struct Flaggable {
		/// @brief Flags attributed to the object.
		usize flags = 0;

		/// @brief Sets or clears a series of flags.
		/// @param mask Flags to set.
		/// @param state Whether to set or clear them. By default it is `true`.
		/// @return Reference to self.
		constexpr Flaggable& setFlags(usize const mask, bool const state = true) {
			if (state)	flags |= mask;
			else		flags &= ~mask;
			return *this;
		}

		/// @brief Returns whether one or more flags in a given set of flags are set.
		/// @param mask Flags to check.
		/// @return Whether one or more are set.
		constexpr bool areAnyFlagsSet(usize const mask) const {
			return flags & mask;
		}

		/// @brief Returns whether all flags in a set of flags are set.
		/// @param mask Flags to check.
		/// @return Whether all are set.
		constexpr bool areAllFlagsSet(usize const mask) const {
			return (flags & mask) == mask;
		}
	};

	/// @brief Collidable object configuration.
	/// @tparam C Collider configuration default value.
	/// @tparam L Collision layer configuration default value.
	/// @tparam M Collision mask default value.
	template<ColliderConfig C, CollisionLayerConfig L, auto M>
	struct CollisionObjectConfig {
		/// @brief Collision mask settings type.
		using CollisionMaskConfig = decltype(M);
		/// @brief Collider settings.
		ColliderConfig const		colli	= C;
		/// @brief Collision layer settings.
		CollisionLayerConfig const	layer	= L;
		/// @brief Collision mask settings.
		CollisionMaskConfig const	mask	= M;

		/// @brief Constructs the configuration.
		/// @param colli Collisder settings.
		/// @param layer Collision layer settings.
		/// @param mask Collison mask settings.
		constexpr CollisionObjectConfig(
			ColliderConfig const& colli			= C,
			CollisionLayerConfig const& layer	= L,
			CollisionMaskConfig const& mask		= M
		): colli(colli), layer(layer), mask(mask) {}

		/// @brief Copy constructor (defaulted).
		constexpr CollisionObjectConfig(CollisionObjectConfig const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr CollisionObjectConfig(CollisionObjectConfig&& other)		= default;

		/// @brief Copy assignment operator (defaulted).
		constexpr CollisionObjectConfig& operator=(CollisionObjectConfig const& other)	= default;
		/// @brief Move assignment operator (defaulted).
		constexpr CollisionObjectConfig& operator=(CollisionObjectConfig&& other)		= default;
	};

	/// @brief Sprite-mesh-referencing object component.
	struct ReferencesSpriteMesh {
		/// @brief Main sprites container.
		Graph::ReferenceHolder& mainMesh;
	};

	/// @brief Glow-sprite-mesh-referencing object component.
	struct ReferencesGlowSpriteMesh	{
		/// @brief Glow sprites container.
		Graph::ReferenceHolder& glowMesh;
	};

	/// @brief Game-bounds-referencing object component.
	struct ReferencesGameBounds {
		/// @brief Game board.
		GameArea& board;
		/// @brief Game playfield.
		GameArea& playfield;
	};
}

#endif