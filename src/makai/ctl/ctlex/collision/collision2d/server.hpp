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
		using ColliderType = typename CollisionServer<SI>::Collider;

		/// @brief Server collision object.
		struct Collider: Area {
			/// @brief Destructor.
			constexpr ~Collider() {CollisionServer::unbind(this);}

			struct IData {
				virtual ~IData() {}
			};

			using CollisionEvent = Functor<void(Collider const&, Direction const)>;

			/// @brief
			///		Checks collision between objects,
			///		and fires the appropriate collision events,
			///		depending on its result.
			/// @param other `Collider` to check against.
			constexpr void process(Collider const& other) const {
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
			/// @param other `Collider` to check against.
			template<usize SI>
			constexpr void process(ColliderType<SI> const& other) const {
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

			/// @brief Event to fire on collision.
			/// @param collider Collider that this object collided with.
			/// @param direction Collision direction.
			CollisionEvent onCollision;
			
			/// @brief Unique collider ID.
			usize const ID;

			/// @brief data associated with the collider.
			Reference<IData const> data;

		private:
			template <usize> friend class CollisionServer;

			/// @brief Amount of created colliders.
			inline static usize count = 0;

			/// @brief Copy constructor (deleted). 
			constexpr Collider(Collider const& other)	= delete;
			/// @brief Move constructor (deleted).
			constexpr Collider(Collider&& other)		= delete;

			/// @brief Default constructor.
			constexpr Collider(): ID(++count)								{CollisionServer::bind(this);}
			/// @brief Constructs the collider from a collision area.
			/// @param other Collision area to construct from.
			constexpr Collider(Area const& other): Area{other}, ID(++count)	{CollisionServer::bind(this);}
		};

		struct Layer {
			LayerMask	affects;
			LayerMask	affectedBy;

			[[nodiscard]] constexpr Unique<Collider> createCollider() {
				auto colli = new Collider();
				colliders.pushBack(colli);
				return Unique<Collider>(colli);
			}

			[[nodiscard]] constexpr Unique<Collider> createCollider(Area const& area) {
				auto colli = new Collider(area);
				colliders.pushBack(colli);
				return Unique<Collider>(colli);
			}

			constexpr void process(Layer const& other) const {
				Direction dir = asDirection(
					affects.match(other.affectedBy).overlap(),
					affectedBy.match(other.affects).overlap()
				);
				if (dir == Direction::CD_NONE) return;
				for (auto const& a: colliders)
					for (auto const& b: other.colliders)
						if (a != b) a->process(*b);
			}

		private:
			template <usize> friend class Server;

			List<Collider*>	colliders;
		};

		/// @brief Default constructor.
		constexpr CollisionServer() {}

		/// @brief Handles collision between a given collider, and a set of layers.
		/// @param area Collider to check.
		/// @param layers Layers to check against.
		template<usize SI>
		constexpr static void check(ColliderType<SI> const& area, LayerMask const& layers) {
			if (!area.affects.match(layers).overlap() || !area.canCollide) return;
			for (Collider* c : colliders)
				if (c->enabled && c->affectedBy.match(layers).overlap())
					area.process(*c);
		}

		/// @brief Handles for a given collider.
		/// @param area Collider to check.
		template<usize SI>
		constexpr static void check(ColliderType<SI> const& area) {
			for (Collider* c : colliders)
				area.process(*c);
		}

		[[nodiscard]] constexpr static Unique<Collider> createCollider(usize const layer) {
			return layers[layer].createCollider();
		}

		[[nodiscard]] constexpr static Unique<Collider> createCollider(Area const& area, usize const layer) {
			return layers[layer].createCollider(area);
		}

		/// @brief Processes (and handles) collision for all colliders in the server.
		constexpr static void process() {
			for (usize i = 0; i < 64; ++i)
				for (usize j = i; j < 64; ++j)
					layers[i].process(layers[j]);
		}

		static inline As<Layer[64]> layers = {};

	private:
		friend class Unique<Collider>;

		constexpr static void bind(Collider* const collider) {
			colliders.pushBack(collider);
		}

		constexpr static void unbind(Collider* const collider) {
			colliders.eraseLike(collider);
			for (auto& layer: layers) layer.colliders.eraseLike(collider);
		}

		friend class Collider;

		/// @brief Server collider objects.
		inline static List<Collider*> colliders;

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
	using Collider = typename CollisionServer<I>::Collider;
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_SERVER_H
