#ifndef MAKAILIB_EX_GAME_DANMAKU_BULLET_H
#define MAKAILIB_EX_GAME_DANMAKU_BULLET_H

#include "core.hpp"
#include "server.hpp"

/// @brief Danmaku-specific game extensions.
namespace Makai::Ex::Game::Danmaku {
	/// @brief Bullet server bullet.
	struct Bullet;
	/// @brief Bullet configuration.
	struct BulletConfig;

	/// @brief Bullet server.
	/// @tparam T Bullet type. By default, it is `Bullet`.
	/// @tparam C Bullet configuration type. By default, it is `BulletConfig`.
	template<class T = Bullet, class C = BulletConfig> struct BulletServer;

	/// @brief Bullet configuration.
	struct BulletConfig: ServerObjectConfig, GameObjectConfig {
		using GameObjectConfig::CollisionMask;
		/// @brief Collision masks & tags.
		struct Collision {
			/// @brief Bullet eraser mask.
			CollisionMask const eraser	= Danmaku::Collision::Mask::BULLET_ERASER;
			/// @brief Player tag.
			CollisionMask const player	= Danmaku::Collision::Tag::FOR_PLAYER_1;
		} const mask = {};
	};
	
	/// @brief Bullet server bullet.
	struct Bullet: AServerObject, ASpriteContainer, AttackObject, Circular, Glowing, Dope, RotatesSprite {
		/// @brief Constructs the bullet.
		/// @param cfg Bullet configuration to use.
		Bullet(BulletConfig const& cfg):
			AServerObject(cfg), mask(cfg.mask), server(cfg.server) {
			collision()->shape = shape.template as<C2D::IBound2D>();
		}

		/// @brief Destructor.
		virtual ~Bullet() {}

		/// @brief Resets all of the object's properties to their default values.
		/// @return Reference to self.
		Bullet& clear() override {
			AServerObject::clear();
			rotateSprite	= true;
			glowOnSpawn		= true;
			dope			= true;
			radius			= {1};
			velocity		= {};
			rotation		= {};
			sprite			= {};
			damage			= {};
			glow			= {};
			autoDecay		= false;
			bouncy			= false;
			loopy			= false;
			grazed			= false;
			animColor		= Graph::Color::WHITE;
			spawnglow		= 0;
			spawnsize		= 0;
			counter			= 0;
			initSprites();
			return *this;
		}

		/// @brief Restarts the object's transformable properties to the beginning.
		/// @return Reference to self.
		Bullet& reset() override {
			if (isFree()) return *this;
			AServerObject::reset();
			velocity.factor	= 0;
			rotation.factor	= 0;
			radius.factor	= 0;
			scale.factor	= 0;
			return *this;
		}

		/// @brief Executes every update cycle.
		void onUpdate(float delta) override {
			if (isFree()) return;
			AServerObject::onUpdate(delta);
			updateSprite(mainSprite.reference());
			updateSprite(glowSprite.reference(), true);
			updateHitbox();
			animate();
			if (paused()) return;
			color.next();
			radius.next();
			glow.next();
			if (autoDecay) damage.next();
			trans.position	+= Math::angleV2(rotation.next()) * velocity.next() * delta;
			trans.rotation	= rotation.value;
			trans.scale		= scale.next();
			playfieldCheck();
			loopAndBounce();
		}

		/// @brief Discards the object, if applicable.
		/// @param immediately Whether to despawn first, or discard directly. By default, it is `false`.
		/// @param force Whether to force discard. By default, it is `false`.
		/// @return Reference to self.
		Bullet& discard(bool const immediately = false, bool const force = false) override {
			if (isFree()) return *this;
			if (!discardable && !force) return *this;
			if (!immediately)	despawn();
			else				free();
			return *this;
		}

		/// @brief Spawns the object.
		/// @return Reference to self.
		Bullet& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = State::SOS_SPAWNING;
			onAction(*this, Action::SOA_SPAWN_BEGIN);
			spawnglow = 0;
			animColor.a = 0;
			spawnsize = 1.0;
			counter = 0;
			return *this;
		}

		/// @brief Despawns the object.
		/// @return Reference to self.
		Bullet& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = State::SOS_DESPAWNING;
			onAction(*this, Action::SOA_DESPAWN_BEGIN);
			return *this;
		}

		/// @brief Executes when a collision event happens.
		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
			if (
				collider.getLayer().affects & mask.eraser
			&&	collider.tags & mask.player
			)
				discard();
		}

		/// @brief Sets the sprite's current rotation.
		/// @param angle Rotation angle.
		/// @return Reference to self.
		Bullet& setSpriteRotation(float const angle) override {
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
		Bullet& setFree(bool const state) override {
			setCollisionState(false);
			active = state;
			if (state) {
				objectState = State::SOS_FREE;
				hideSprites();
				clear();
				release(this, server);
			} else {
				showSprites();
				objectState = State::SOS_ACTIVE;
			}
			return *this;
		}
		
		/// @brief Whether the bullet should bounce when touching the edge of the board. Only bounces once.
		bool bouncy	= false;
		/// @brief Whether the bullet should wrap around when leaving one edge of the board. Only loops once.
		bool loopy	= false;

		/// @brief Whether the bullet has been grazed.
		bool grazed	= false;

		/// @brief Collision mask associated with the bullet.
		BulletConfig::Collision const mask;

	private:
		/// @brief Server associated with the object.
		AServer&	server;

		/// @brief Main sprite.
		TileHolder mainSprite	= TileHolder(nullptr);
		/// @brief Glow sprite.
		TileHolder glowSprite	= TileHolder(nullptr);

		/// @brief Counter used for spawn/despawn timing purposes.
		usize counter	= 0;
		/// @brief Current spawn glow.
		float spawnglow	= 0;
		/// @brief Current spawn size.
		float spawnsize = 1;

		/// @brief Spawn size factor.
		constexpr static float SPAWN_GROWTH = .5;

		/// @brief Current animation color.
		Vector4 animColor = Graph::Color::WHITE;

		/// @brief Collision shape.
		Instance<C2D::Circle> shape = new C2D::Circle(0);

		void playfieldCheck() {
			if (dope && !shape->aabb().overlap(playfield.aabb())) free();
		}

		void loopAndBounce() {
			if (bouncy && !board.aabb().contains(trans.position)) {
				auto const
					min = board.min(),
					max = board.max()
				;
				if (trans.position.x < min.x) shift(PI);
				if (trans.position.x > max.x) shift(PI);
				if (trans.position.y < min.y) shift(0);
				if (trans.position.y > max.y) shift(0);
				onAction(*this, Action::SOA_BOUNCE);
				bouncy = false;
			} else if (loopy && shape && !shape->aabb().overlap(board.aabb())) {
				auto const
					min = board.min(),
					max = board.max()
				;
				if (trans.position.x < min.x) trans.position.x = max.x + shape->radius.max()*2;
				if (trans.position.x > max.x) trans.position.x = min.x - shape->radius.max()*2;
				if (trans.position.y < min.y) trans.position.y = max.y + shape->radius.max()*2;
				if (trans.position.y > max.y) trans.position.y = min.y - shape->radius.max()*2;
				onAction(*this, Action::SOA_LOOP);
				loopy = false;
			}
		}

		void shift(float const angle) {
			rotation.value	= angle - rotation.value;
			rotation.start	= angle - rotation.start;
			rotation.stop	= angle - rotation.stop;
		}

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

		void updateSprite(Reference<Tile> const& sprite, bool glowSprite = false) {
			if (!sprite) return;
			sprite->visible = true;
			sprite->tile	= this->sprite.tile;
			sprite->size	= this->sprite.sheetSize;
			if (rotateSprite)
				sprite->local.rotation.z	= trans.rotation;
			sprite->local.position			= Vec3(trans.position, sprite->local.position.z);
			sprite->local.scale				= trans.scale * spawnsize;
			float const iglow = glowOnSpawn ? Math::lerp<float>(1, glow.value, spawnglow) : glow.value;
			auto const glowFX = Graph::Color::alpha(glowSprite ? iglow : 1 - iglow);
			sprite->setColor(animColor * color.value * glowFX);
		}

		void updateHitbox() {
			if (shape) {
				shape->radius	= radius.value * trans.scale;
				shape->position	= trans.position;
				shape->rotation	= trans.rotation;
			}
		}

		void animate() {
			switch (objectState) {
				case State::SOS_DESPAWNING: {
					if (counter++ < despawnTime) {
						spawnglow = counter / static_cast<float>(despawnTime);
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
					//	spawnsize = 1.0 + animColor.a * SPAWN_GROWTH;
					} else {
						spawnglow = 0;
						animColor.a = 0;
						spawnsize = 1.0;
						counter = 0;
						onAction(*this, Action::SOA_DESPAWN_END);
						free();
					}
				} break;
				case State::SOS_SPAWNING: {
					if (counter++ < spawnTime) {
						spawnglow = 1.0 - counter / static_cast<float>(spawnTime);
						animColor.a = counter / static_cast<float>(spawnTime);
						spawnsize = (1.0 + SPAWN_GROWTH) - animColor.a;
					} else {
						spawnglow = 0;
						animColor.a = 1;
						spawnsize = 1.0;
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

		Bullet(Bullet const& other)	= default;
		Bullet(Bullet&& other)		= default;

		template <class, class> friend class BulletServer;
	};

	/// @brief Bullet collision configuration.
	struct BulletCollisionConfig: CollisionObjectConfig<
		ColliderConfig{
			Collision::Layer::ENEMY_BULLET,
			Collision::Tag::FOR_PLAYER_1
		},
		CollisionLayerConfig{
			Collision::Mask::ENEMY_BULLET,
			Collision::Mask::BULLET_ERASER
		},
		BulletConfig::Collision{}
	> {
		using CollisionObjectConfig::CollisionObjectConfig;
	};

	/// @brief Bullet server configuration.
	struct BulletServerConfig:
		ServerConfig,
		ServerMeshConfig,
		ServerGlowMeshConfig,
		BoundedObjectConfig,
		BulletCollisionConfig {};

	/// @brief Bullet server instance configuration.
	struct BulletServerInstanceConfig: ServerConfig, BulletCollisionConfig {};

	/// @brief Bullet server.
	/// @tparam TBullet Bullet type. By default, it is `Bullet`.
	/// @tparam TConfig Bullet configuration type. By default, it is `BulletConfig`.
	template<Type::Derived<Bullet> TBullet, Type::Derived<BulletConfig> TConfig>
	struct BulletServer<TBullet, TConfig>:
		AServer,
		AUpdateable,
		ReferencesSpriteMesh,
		ReferencesGlowSpriteMesh,
		ReferencesGameBounds {
		/// @brief Collision mask type.
		using CollisionMask = AGameObject::CollisionMask;

		/// @brief Bullet type.
		using BulletType = TBullet;
		/// @brief Bullet configuration type.
		using ConfigType = TConfig;

		/// @brief Constructs the bullet server.
		/// @param cfg Bullet server configuration.
		BulletServer(BulletServerConfig const& cfg):
			ReferencesSpriteMesh{cfg},
			ReferencesGlowSpriteMesh{cfg},
			ReferencesGameBounds{cfg} {
			auto& cl		= CollisionServer::layers[cfg.colli.layer];
			cl.affects		= cfg.layer.affects;
			cl.affectedBy	= cfg.layer.affectedBy;
			all.resize(cfg.capacity);
			free.resize(cfg.capacity);
			used.resize(cfg.capacity);
			for (usize i = 0; i < cfg.capacity; ++i) {
				float const zoff = i / static_cast<float>(cfg.capacity);
				all.constructBack(ConfigType{*this, cfg, cfg.colli, cfg.mask});
				all.back().mainSprite = mainMesh.createReference<Graph::Ref::TilePlane>();
				all.back().mainSprite->local.position.z = -zoff;
				if (&cfg.mainMesh != &cfg.glowMesh) {
					all.back().glowSprite = glowMesh.createReference<Graph::Ref::TilePlane>();
					all.back().glowSprite->local.position.z = -zoff;
				}
				all.back().hideSprites();
				all.back().setCollisionState(false);
				free.pushBack(&all.back());
			}
		}
		
		/// @brief Tries to acquire a bullet.
		/// @return Reference to bullet, or `nullptr`.
		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Reference<BulletType> bullet = b.template as<BulletType>();
				bullet->clear();
				bullet->enable();
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

		/// @brief Discards all active bullets, if applicable.
		void discardAll() override {
			auto const uc = used;
			for (auto b: uc) {
				BulletType& bullet = access<BulletType>(b);
				bullet.discard();
			};
		}
		
		/// @brief Frees all active bullets.
		void freeAll() override {
			auto const uc = used;
			for (auto b: uc) {
				BulletType& bullet = access<BulletType>(b);
				bullet.free();
			};
		}

		/// @brief Despaws all active bullets.
		void despawnAll() override {
			auto const uc = used;
			for (auto b: uc) {
				BulletType& bullet = access<BulletType>(b);
				bullet.despawn();
			};
		}

		/// @brief Returns the server's bullet capacity.
		/// @return Bullet capacity.
		usize capacity() override {
			return all.size();
		}

		/// @brief Returns all active bullets in a given area.
		/// @param bound Area to get bullets in.
		/// @return Active bullets in area.
		ObjectQueryType getInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				BulletType& bullet = access<BulletType>(b);
				if (
					bullet.shape
				&&	C2D::withinBounds(*bullet.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

		/// @brief Returns all active bullets not in a given area.
		/// @param bound Area to get bullets not in.
		/// @return Active bullets not in area.
		ObjectQueryType getNotInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				BulletType& bullet = access<BulletType>(b);
				if (
					bullet.shape
				&&	!C2D::withinBounds(*bullet.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

	protected:
		/// @brief Returns whether a bullet is in the active bullets list.
		/// @param object Bullet to check.
		/// @return Whether bullet exists in active list.
		bool contains(HandleType const& object) override {
			return (used.find(object) != -1);
		}

		/// @brief Frees up a bullet from use.
		/// @param object Bullet to free.
		/// @return Reference to self.
		BulletServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			BulletType& bullet = *(object.template as<BulletType>());
			if (!bullet.isFree()) bullet.free();
			AServer::release(object);
			return *this;
		}

	private:
		/// @brief All items in the server.
		StaticList<BulletType> all;
	};
}

#endif