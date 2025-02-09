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
	/// @tparam I Server ID.
	/// @tparam L Collision layer count.
	template<usize I, usize L = 64>
	struct CollisionServer {
		/// @brief Server ID.
		constexpr static usize ID = I;

		/// @brief Max layers count.
		constexpr static usize MAX_LAYERS = L;

		/// @brief Other server collider type.
		/// @tparam SI Server ID.
		template<usize SI, usize SL = MAX_LAYERS>
		using ColliderType = typename CollisionServer<SI, SL>::Collider;

		struct Layer;

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
			/// @param direction Direction to process collision for.
			constexpr void process(Collider const& other, Direction const direction) const {
				if (!colliding(other)) return;
				switch(direction) {
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
			/// @param direction Direction to process collision for.
			template<usize SI, usize SL>
			constexpr void process(ColliderType<SI, SL> const& other, Direction const direction) const {
				if (!colliding(other)) return;
				switch(direction) {
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

			/// @brief Returns the layer associated with this collision object.
			/// @return Associated layer.
			Layer& getLayer() const {return CollisionServer::layers[layerID];}

			/// @brief Sets the layer associated with this collision object.
			/// @return Reference to self.
			Collider& setLayer(usize const layer) {
				if (layer < MAX_LAYERS) {
					getLayer().colliders.eraseLike(this);
					layerID = layer;
					getLayer().colliders.pushBack(this);
				}
				return *this;
			}

		private:
			template <usize> friend class CollisionServer;

			/// @brief Layer the collider resides in.
			usize layerID;

			/// @brief Amount of created colliders.
			inline static usize count = 0;

			/// @brief Copy constructor (deleted). 
			constexpr Collider(Collider const& other)	= delete;
			/// @brief Move constructor (deleted).
			constexpr Collider(Collider&& other)		= delete;

			/// @brief Default constructor.
			constexpr Collider(
				usize const layer
			): ID(++count), layerID(layer)				{CollisionServer::bind(this);}
			/// @brief Constructs the collider from a collision area.
			/// @param other Collision area to construct from.
			constexpr Collider(
				Area const& other,
				usize const layer
			): Area{other}, ID(++count), layerID(layer)	{CollisionServer::bind(this);}
		};

		struct Layer {
			LayerMask	affects;
			LayerMask	affectedBy;

			[[nodiscard]] constexpr Unique<Collider> createCollider() {
				auto colli = new Collider(*this);
				colliders.pushBack(colli);
				return Unique<Collider>(colli);
			}

			[[nodiscard]] constexpr Unique<Collider> createCollider(Area const& area) {
				auto colli = new Collider(area, *this);
				colliders.pushBack(colli);
				return Unique<Collider>(colli);
			}

			/// @brief Checks collision direction between two layers.
			/// @param a `Layer` to check.
			/// @param b `Layer` to check against.
			/// @return Collision event direction.
			/// @note Directions:
			///
			///		- Forward:	A --> B
			///
			///		- Backward:	A <-- B
			///
			///		- Both:		A <-> B
			constexpr static Direction check(Layer const& a, Layer const& b) {
				return asDirection(
					a.affects.match(b.affectedBy).overlap(),
					b.affectedBy.match(a.affects).overlap()
				);
			}

			/// @brief Checks collision direction between two layers.
			/// @param a `Layer` to check.
			/// @param b `Layer` to check against.
			/// @return Collision event direction.
			/// @note Directions:
			///
			///		- Forward:	A --> B
			///
			///		- Backward:	A <-- B
			///
			///		- Both:		A <-> B
			constexpr friend Direction operator<=>(Layer const& a, Layer const& b) {
				return check(a, b);
			}

			constexpr void process(Layer const& other) const {
				if (colliders.empty() || other.colliders.empty()) return;
				const auto dir = check(*this, other);
				if (dir == Direction::CD_NONE) return;
				for (auto const& a: colliders)
					for (auto const& b: other.colliders)
						if (a != b) a->process(*b, dir);
			}

		private:
			template <usize> friend class CollisionServer;

			List<Collider*>	colliders;
		};

		/// @brief Default constructor.
		constexpr CollisionServer() {}

		/// @brief Handles collision between a given collider, and a set of layers.
		/// @param area Collider to check.
		/// @param layers Layers to check against.
		template<usize SI, usize SL>
		constexpr static void check(ColliderType<SI, SL> const& area, LayerMask const& layers) {
			if (!area.affects.match(layers).overlap() || !area.canCollide) return;
			for (Collider* c : colliders)
				if (c->enabled && c->layer.affectedBy.match(layers).overlap())
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
			for (usize i = 0; i < MAX_LAYERS; ++i)
				for (usize j = i; j < MAX_LAYERS; ++j)
					layers[i].process(layers[j]);
		}

		static inline As<Layer[MAX_LAYERS]> layers = {};

	private:
		friend class Unique<Collider>;

		constexpr static void bind(Collider* const collider) {
			colliders.pushBack(collider);
			collider->getLayer().colliders.pushBack(collider);
		}

		constexpr static void unbind(Collider* const collider) {
			colliders.eraseLike(collider);
			collider->getLayer().colliders.eraseLike(collider);
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
	using Server = CollisionServer<0, 64>;

	/// @brief Server collision object interface.
	/// @tparam I Server ID. 
	template<usize I = 0, usize L = 64>
	using Collider = typename CollisionServer<I, L>::Collider;
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_SERVER_H
