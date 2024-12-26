#ifndef CTL_EX_COLLISION_COLLISION2D_SERVER_H
#define CTL_EX_COLLISION_COLLISION2D_SERVER_H

#include "area.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Two-dimensional collision.
namespace Collision::C2D {
	namespace {
		using
			Math::center,
			Math::Transform2D
		;
	}

	/// @brief Collision server.
	/// @tparam Server ID.
	template<usize I>
	struct CollisionServer {
		/// @brief Server ID.
		constexpr static usize ID = I;

		/// @brief Other server collider type.
		/// @tparam SI Server ID.
		template<usize SI>
		using IColliderType = typename CollisionServer<SI>::ICollider;

		/// @brief Server collision object interface.
		struct ICollider: Area {
			/// @brief Destructor.
			constexpr virtual ~ICollider() {CollisionServer::unbind(this);}

			/// @brief Copy constructor (deleted). 
			constexpr ICollider(ICollider const& other)	= delete;
			/// @brief Move constructor (deleted).
			constexpr ICollider(ICollider&& other)		= delete;

			/// @brief Empty constructor.
			constexpr ICollider()								{CollisionServer::bind(this);}
			/// @brief Constructs the collider from a collision area.
			/// @param other Collision area to construct from.
			constexpr ICollider(Area const& other): Area{other}	{CollisionServer::bind(this);}

			/// @brief
			///		Checks collision between objects,
			///		and fires the appropriate collision events,
			///		depending on its result.
			/// @param other `ICollider` to check against.
			constexpr void process(ICollider const& other) const {
				switch(colliding(other)) {
					using enum Direction;
					default:
					case CD_NONE: break;
					case CD_FORWARDS:
						other.onCollision(*this, CD_FORWARDS);
					break;
					case CD_BACKWARDS:
						onCollision(other, CD_BACKWARDS);
					break;
					case CD_BOTH:
						other.onCollision(*this, CD_BOTH);
						onCollision(other, CD_BOTH);
					break;
				}
			}


			/// @brief
			///		Checks collision between objects,
			///		and fires the appropriate collision events,
			///		depending on its result.
			/// @param other `ICollider` to check against.
			template<usize SI>
			constexpr void process(IColliderType<SI> const& other) const {
				switch(colliding(other)) {
					using enum Direction;
					default:
					case CD_NONE: break;
					case CD_FORWARDS:
						other.onCollision(*this, CD_FORWARDS);
					break;
					case CD_BACKWARDS:
						onCollision(other, CD_BACKWARDS);
					break;
					case CD_BOTH:
						other.onCollision(*this, CD_BOTH);
						onCollision(other, CD_BOTH);
					break;
				}
			}

			/// @brief Event to fire on collision. Must be implemented.
			/// @param collider Collider that this object collided with.
			/// @param direction Collision direction.
			virtual void onCollision(ICollider const& collider, Direction const& direction) const = 0;
		};

		/// @brief Empty constructor.
		constexpr CollisionServer() {}

		/// @brief Handles collision between a given collider, and a set of layers.
		/// @param area Collider to check.
		/// @param layers Layers to check against.
		template<usize SI>
		constexpr static void check(IColliderType<SI> const& area, LayerMask const& layers) {
			if (!area.affects.match(layers).overlap()|| !area.enabled) return;
			for (ICollider* c : colliders)
				if (c->enabled && c->affectedBy.match(layers).overlap())
					area.process(*c);
		}

		/// @brief Handles for a given collider.
		/// @param area Collider to check.
		template<usize SI>
		constexpr static void check(IColliderType<SI> const& area) {
			for (ICollider* c : colliders)
				area.process(*c);
		}

		/// @brief Processes (and handles) collision for all colliders in the server.
		constexpr static void process() {
			usize const stop = colliders.size();
			for (usize start = 1; start < stop; ++start) {
				ICollider& c = *colliders[start-1];
				for (usize i = start; i < stop; ++i)
					c.process(*colliders[start]);
			}
		}

	private:
		constexpr static void bind(ICollider* const collider) {
			colliders.pushBack(collider);
		}

		constexpr static void unbind(ICollider* const collider) {
			colliders.eraseLike(collider);
		}

		friend class ICollider;

		/// @brief Server collider objects.
		inline static List<ICollider*> colliders;

		void *operator new(usize);
		void operator delete(pointer);
		void *operator new[](usize);
		void operator delete[](pointer);
	};
	/// @brief Default collision server.
	using Server = CollisionServer<0>;

	/// @brief Server collision object interface.
	/// @tparam I Server ID. 
	template<usize I = 0>
	using ICollider = typename CollisionServer<I>::ICollider;
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_SERVER_H
