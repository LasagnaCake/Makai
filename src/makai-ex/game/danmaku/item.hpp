#ifndef MAKAILIB_EX_GAME_DANMAKU_ITEM_H
#define MAKAILIB_EX_GAME_DANMAKU_ITEM_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	/// @brief Item server item.
	struct Item;
	/// @brief Item configuration.
	struct ItemConfig;

	/// @brief Item server.
	/// @tparam T Item type. By default, it is `Item`.
	/// @tparam C Item configuration type. By default, it is `ItemConfig`.
	template<class T = Item, class C = ItemConfig> struct ItemServer;

	/// @brief Item configuration.
	struct ItemConfig: ServerObjectConfig, GameObjectConfig {
		using GameObjectConfig::CollisionMask;
		/// @brief Collision masks & tags.
		struct Collision {
			/// @brief Player tag.
			CollisionMask const player = Danmaku::Collision::Tag::FOR_PLAYER_1;
		} const mask;
	};

	/// @brief Item server item.
	struct Item: AServerObject, ISpriteContainer, Weighted, Circular, Glowing, Dope, RotatesSprite, Magnetizable {
		/// @brief Constructs the item.
		/// @param cfg Item configuration to use.
		Item(ItemConfig const& cfg):
			AServerObject(cfg), mask(cfg.mask), server(cfg.server) {
			collision()->shape = shape.template as<C2D::IBound2D>();
		}

		/// @brief Destructor.
		virtual ~Item() {}

		/// @brief Resets all of the object's properties to their default values.
		/// @return Reference to self.
		Item& clear() override {
			AServerObject::clear();
			rotateSprite		= true;
			dope				= true;
			jumpy				= false;
			glowOnSpawn			= false;
			radius				= {1};
			sprite				= {};
			gravity				= {1};
			terminalVelocity	= {1};
			magnet				= {};
			glow				= {};
			id					= 0;
			value				= 1;
			animColor			= Graph::Color::WHITE;
			counter				= 0;
			spawnglow			= 0;
			acceleration		= 0;
			internalRotation	= 0;
			initSprites();
			return *this;
		}

		/// @brief Restarts the object's transformable properties to the beginning.
		/// @return Reference to self.
		Item& reset() override {
			if (isFree()) return *this;
			AServerObject::reset();
			radius.factor			= 0;
			scale.factor			= 0;
			gravity.factor			= 0;
			terminalVelocity.factor	= 0;
			magnet.strength.factor	= 0;
			return *this;
		}

		/// @brief Executes every update cycle.
		void onUpdate(float delta) override {
			if (isFree()) return;
			AServerObject::onUpdate(delta);
			updateSprite(mainSprite.asWeak());
			updateSprite(glowSprite.asWeak(), true);
			updateHitbox();
			animate();
			if (paused()) return;
			color.next();
			radius.next();
			glow.next();
			terminalVelocity.next();
			acceleration += gravity.next();
			if (!jumpy) {
				auto const tva = terminalVelocity.value.absolute();
				acceleration.clamp(-tva, tva);
			}
			if (magnet.enabled && magnet.target && objectState == State::SOS_ACTIVE)
				trans.position += trans.position.normalTo(*magnet.target) * magnet.strength.next() * delta;
			else if (jumpy) {
				auto const tva = terminalVelocity.value.absolute();
				if (acceleration.x > +tva.x) acceleration.x = (acceleration.x * -1) - gravity.value.x;
				if (acceleration.y > +tva.y) acceleration.y = (acceleration.y * -1) - gravity.value.y;
				if (acceleration.x < -tva.x) acceleration.x = (acceleration.x * -1) + gravity.value.x;
				if (acceleration.y < -tva.y) acceleration.y = (acceleration.y * -1) + gravity.value.y;
				trans.position += acceleration * delta;
			} else trans.position += acceleration * delta;
			trans.scale		= scale.next();
			playfieldCheck();
		}

		/// @brief Discards the object, if applicable.
		/// @param immediately Whether to despawn first, or discard directly. By default, it is `false`.
		/// @param force Whether to force discard. By default, it is `false`.
		/// @return Reference to self.
		Item& discard(bool const immediately = false, bool const force = false) override {
			if (isFree()) return *this;
			if (!discardable && !force) return *this;
			if (!immediately)	despawn();
			else				free();
			return *this;
		}

		/// @brief Spawns the object.
		/// @return Reference to self.
		Item& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			animColor.a = 0;
			counter = 0;
			internalRotation = 0;
			objectState = State::SOS_SPAWNING;
			onAction(*this, Action::SOA_SPAWN_BEGIN);
			return *this;
		}

		/// @brief Despawns the object.
		/// @return Reference to self.
		Item& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			animColor.a = 1;
			counter = 0;
			internalRotation = 0;
			objectState = State::SOS_DESPAWNING;
			onAction(*this, Action::SOA_DESPAWN_BEGIN);
			return *this;
		}

		/// @brief Executes when a collision event happens.
		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
		}

		/// @brief Sets the sprite's current rotation.
		/// @param angle Rotation angle.
		/// @return Reference to self.
		Item& setSpriteRotation(float const angle) override {
			if (isFree()) return *this;
			if (mainSprite)
				mainSprite->local.rotation.z	= angle;
			if (glowSprite)
				glowSprite->local.rotation.z	= angle;
			return *this;
		}

		/// @brief Returns the sprite's current rotation.
		/// @return Sprite rotation.
		float getSpriteRotation() const override {
			if (isFree()) return 0;
			if (mainSprite)
				return mainSprite->local.rotation.z;
			if (glowSprite)
				return glowSprite->local.rotation.z;
			return 0;
		}

		/// @brief Sets the object's "free state".
		/// @param state Whether to set the object as free or as active.
		/// @return Reference to self.
		Item& setFree(bool const state) override {
			if (state) {
				active = false;
				hideSprites();
				objectState = State::SOS_FREE;
				clear();
				release(this, server);
			} else {
				setCollisionState(false);
				active = true;
				showSprites();
				objectState = State::SOS_ACTIVE;
			}
			return *this;
		}
		
		/// @brief Collision mask associated with the item.
		ItemConfig::Collision const mask;
		
		/// @brief The item's ID.
		usize id	= 0;
		/// @brief The item's value.
		usize value	= 1;

		/// @brief Whether the item bounces in place, instead of falling down.
		bool jumpy = false;

	private:
		/// @brief Server associated with the object.
		AServer&	server;

		/// @brief Internal rotation.
		float		internalRotation = 0;

		/// @brief Main sprite.
		SpriteInstance mainSprite	= nullptr;
		/// @brief Glow sprite.
		SpriteInstance glowSprite	= nullptr;

		/// @brief Item acceleration.
		Vector2 acceleration = 0;

		/// @brief Counter used for spawn/despawn timing purposes.
		usize counter	= 0;
		/// @brief Current spawn glow.
		float spawnglow	= 0;

		/// @brief Current animation color.
		Vector4 animColor = Graph::Color::WHITE;

		/// @brief Collision shape.
		Instance<C2D::Circle> shape = new C2D::Circle(0);

		template <class, class> friend class ItemServer;

		void setSpriteVisibility(bool const setGlowSprite, bool const state) {
			if (glowSprite && setGlowSprite)	glowSprite->visible	= state; 
			else if (mainSprite)				mainSprite->visible	= state;
		}

		void hideSprites() {
			if (glowSprite)	glowSprite->visible	= false; 
			if (mainSprite)	mainSprite->visible	= false;
		}

		void showSprites() {
			if (glowSprite)	glowSprite->visible	= true; 
			if (mainSprite)	mainSprite->visible	= true;
		}
		
		void initSprites() {
			if (mainSprite) mainSprite->local.scale = 0;
			if (glowSprite) glowSprite->local.scale = 0;
		}

		void updateSprite(SpriteHandle const& sprite, bool glowSprite = false) {
			if (!sprite) return;
			sprite->visible = true;
			sprite->frame	= this->sprite.frame;
			sprite->size	= this->sprite.sheetSize;
			if (rotateSprite)
				sprite->local.rotation.z	= trans.rotation + internalRotation;
			sprite->local.position			= Vec3(trans.position, sprite->local.position.z);
			sprite->local.scale				= trans.scale;
			float const iglow = glowOnSpawn ? Math::lerp<float>(1, glow.value, spawnglow) : glow.value;
			auto const glowFX = Graph::Color::alpha(glowSprite ? iglow : 1 - iglow);
			sprite->setColor(animColor * color.value * glowFX);
		}

		void updateHitbox() {
			if (shape) {
				shape->radius	= radius.value * trans.scale;
				shape->position	= trans.position;
				shape->rotation	= trans.rotation + internalRotation;
			}
		}

		void animate() {
			switch (objectState) {
				case State::SOS_DESPAWNING: {
					if (counter++ < despawnTime) {
						spawnglow = counter / static_cast<float>(despawnTime);
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
						internalRotation = (1.0 - counter / static_cast<float>(spawnTime));
						internalRotation *= TAU;
					} else {
						internalRotation = 0;
						counter = 0;
						onAction(*this, Action::SOA_DESPAWN_END);
						free();
					}
				} break;
				case State::SOS_SPAWNING: {
					if (counter++ < spawnTime) {
						spawnglow = 1.0 - counter / static_cast<float>(despawnTime);
						animColor.a = counter / static_cast<float>(spawnTime);
						internalRotation = (counter / static_cast<float>(spawnTime));
						internalRotation *= TAU;
					} else {
						internalRotation = 0;
						counter = 0;
						setCollisionState(true);
						onAction(*this, Action::SOA_SPAWN_END);
						objectState = State::SOS_ACTIVE;
					}
				} break;
				[[likely]]
				default: break;
			}
		}

		void playfieldCheck() {
			if (!dope) return;
			auto const
				min = playfield.min(),
				max = playfield.max()
			;
			if (
				trans.position.x < min.x
			||	trans.position.x > max.x
			||	trans.position.y < min.y
			) free();
		}
	};

	/// @brief Item collision configuration.
	struct ItemCollisionConfig: CollisionObjectConfig<
		ColliderConfig{
			Danmaku::Collision::Layer::ITEM,
			Collision::Tag::FOR_PLAYER_1
		},
		CollisionLayerConfig{
			Danmaku::Collision::Mask::ITEM,
			{}
		},
		ItemConfig::Collision{}
	> {
		using CollisionObjectConfig::CollisionObjectConfig;
	};

	/// @brief Item server configuration.
	struct ItemServerConfig:
		ServerConfig,
		ServerMeshConfig,
		ServerGlowMeshConfig,
		BoundedObjectConfig,
		ItemCollisionConfig {};

	/// @brief Item server instance configuration.
	struct ItemServerInstanceConfig: ServerConfig, ItemCollisionConfig {};

	/// @brief Item server.
	/// @tparam T Item type. By default, it is `Item`.
	/// @tparam C Item configuration type. By default, it is `ItemConfig`.
	template<Type::Derived<Item> TItem, Type::Derived<ItemConfig> TConfig>
	struct ItemServer<TItem, TConfig>:
		AServer,
		AUpdateable,
		ReferencesSpriteMesh,
		ReferencesGlowSpriteMesh,
		ReferencesGameBounds {
		/// @brief Collision mask type.
		using CollisionMask = AGameObject::CollisionMask;
		
		/// @brief Item type.
		using ItemType		= TItem;
		/// @brief Item configuration type.
		using ConfigType	= TConfig;

		/// @brief Constructs the item server.
		/// @param cfg Item server configuration.
		ItemServer(ItemServerConfig const& cfg):
			ReferencesSpriteMesh{cfg},
			ReferencesGlowSpriteMesh{cfg},
			ReferencesGameBounds{cfg} {
			auto& cl		= CollisionServer::layers[cfg.colli.layer];
			cl.affects		= cfg.layer.affects;
			cl.affectedBy	= cfg.layer.affectedBy;
			all.resize(cfg.size);
			free.resize(cfg.size);
			used.resize(cfg.size);
			for (usize i = 0; i < cfg.size; ++i) {
				float const zoff = i / static_cast<float>(cfg.size);
				all.constructBack(ConfigType{*this, cfg, cfg.colli, cfg.mask});
				all.back().mainSprite = mainMesh.createReference<Graph::AnimatedPlaneRef>();
				all.back().mainSprite->local.position.z = -zoff;
				if (&cfg.mainMesh != &cfg.glowMesh) {
					all.back().glowSprite = glowMesh.createReference<Graph::AnimatedPlaneRef>();
					all.back().glowSprite->local.position.z = -zoff;
				}
				all.back().hideSprites();
				all.back().setCollisionState(false);
				free.pushBack(&all.back());
			}
		}

		/// @brief Tries to acquire an item.
		/// @return Reference to item, or `nullptr`.
		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Reference<ItemType> item = b.template as<ItemType>();
				item->clear();
				item->enable();
				return b;
			}
			return nullptr;
		}

		/// @brief Executed every update cycle.
		void onUpdate(float delta, Makai::App& app) override {
			if (used.empty()) return;
			for (auto& obj: all)
				if (!obj.isFree())
					obj.onUpdate(delta);
		}

		/// @brief Discards all active items, if applicable.
		void discardAll() override {
			auto const uc = used;
			for (auto b: uc) {
				ItemType& item = access<ItemType>(b);
				item.discard();
			};
		}
		
		/// @brief Frees all active items.
		void freeAll() override {
			auto const uc = used;
			for (auto b: uc) {
				ItemType& item = access<ItemType>(b);
				item.free();
			};
		}

		/// @brief Despaws all active items.
		void despawnAll() override {
			auto const uc = used;
			for (auto b: uc) {
				ItemType& item = access<ItemType>(b);
				item.despawn();
			};
		}

		/// @brief Returns the server's item capacity.
		/// @return Item capacity.
		usize capacity() override {
			return all.size();
		}

		/// @brief Returns all active items in a given area.
		/// @param bound Area to get items in.
		/// @return Active items in area.
		ObjectQueryType getInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				ItemType& item = access<ItemType>(b);
				if (
					item.shape
				&&	C2D::withinBounds(*item.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

		/// @brief Returns all active items not in a given area.
		/// @param bound Area to get items not in.
		/// @return Active items not in area.
		ObjectQueryType getNotInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				ItemType& item = access<ItemType>(b);
				if (
					item.shape
				&&	!C2D::withinBounds(*item.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

	protected:
		/// @brief Returns whether a item is in the active items list.
		/// @param object Item to check.
		/// @return Whether item exists in active list.
		bool contains(HandleType const& object) override {
			return (used.find(object) != -1);
		}
		
		/// @brief Frees up an item from use.
		/// @param object Item to free.
		/// @return Reference to self.
		ItemServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			ItemType& item = *(object.template as<ItemType>());
			if (!item.isFree()) item.free();
			AServer::release(object);
			return *this;
		}

	private:
		/// @brief All items in the server.
		StaticList<ItemType> all;
	};
}

#endif