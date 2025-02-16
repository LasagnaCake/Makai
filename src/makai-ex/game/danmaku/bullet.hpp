#ifndef MAKAILIB_EX_GAME_DANMAKU_BULLET_H
#define MAKAILIB_EX_GAME_DANMAKU_BULLET_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct Bullet;
	struct BulletConfig;

	template<class T = Bullet, class C = BulletConfig> struct BulletServer;

	struct BulletConfig: ServerObjectConfig, GameObjectConfig {
		using GameObjectConfig::CollisionMask;
		struct Collision {
			CollisionMask const eraser	= Danmaku::Collision::Mask::BULLET_ERASER;
			CollisionMask const player	= Danmaku::Collision::Tag::FOR_PLAYER_1;
		} const mask = {};
	};
	
	struct Bullet: AServerObject, ISpriteContainer, AttackObject, Circular, Glowing, Dope, RotatesSprite {
		Bullet(BulletConfig const& cfg):
			AServerObject(cfg), mask(cfg.mask), server(cfg.server) {
			collision()->shape = shape.template as<C2D::IBound2D>();
		}

		virtual ~Bullet() {}

		Bullet& clear() override {
			AServerObject::clear();
			rotateSprite	= true;
			glowOnSpawn		= true;
			dope			= true;
			radius			= {};
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
			return *this;
		}

		Bullet& reset() override {
			if (isFree()) return *this;
			AServerObject::reset();
			velocity.factor	= 0;
			rotation.factor	= 0;
			radius.factor	= 0;
			scale.factor	= 0;
			return *this;
		}

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
			if (autoDecay) damage.next();
			trans.position	+= Math::angleV2(rotation.next()) * velocity.next() * delta;
			trans.rotation	= rotation.value;
			trans.scale		= scale.next();
			playfieldCheck();
			loopAndBounce();
		}

		Bullet& discard(bool const immediately = false, bool const force = false) override {
			if (isFree()) return *this;
			if (discardable && !force) return *this;
			if (!immediately)	despawn();
			else				free();
			return *this;
		}

		Bullet& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = State::SOS_SPAWNING;
			onAction(*this, Action::SOA_SPAWN_BEGIN);
			return *this;
		}

		Bullet& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = State::SOS_DESPAWNING;
			onAction(*this, Action::SOA_DESPAWN_BEGIN);
			return *this;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
			if (
				collider.getLayer().affects.match(mask.eraser).overlap()
			&&	collider.tags.match(mask.player).overlap()
			)
				discard();
		}

		Bullet& setSpriteRotation(float const angle) override {
			if (isFree()) return *this;
			if (mainSprite)
				mainSprite->local.rotation.z	= angle;
			if (glowSprite)
				glowSprite->local.rotation.z	= angle;
			return *this;
		}

		float getSpriteRotation() const override {
			if (isFree()) return 0;
			if (mainSprite)
				return mainSprite->local.rotation.z;
			if (glowSprite)
				return glowSprite->local.rotation.z;
			return 0;
		}

		Bullet& setFree(bool const state) override {
			if (state) {
				active = false;
				objectState = State::SOS_FREE;
				hideSprites();
				release(this, server);
			} else {
				active = true;
				showSprites();
				objectState = State::SOS_ACTIVE;
			}
			return *this;
		}
		
		bool bouncy	= false;
		bool loopy	= false;

		bool grazed	= false;

		BulletConfig::Collision const mask;

	private:
		AServer&	server;

		SpriteInstance mainSprite	= nullptr;
		SpriteInstance glowSprite	= nullptr;

		usize counter	= 0;
		float spawnglow	= 0;
		float spawnsize = 1;

		constexpr static float SPAWN_GOWTH = .5;

		Vector4 animColor = Graph::Color::WHITE;

		Instance<C2D::Circle> shape = new C2D::Circle(0);

		void playfieldCheck() {
			if (dope && !shape->bounded(playfield.asArea())) free();
		}

		void loopAndBounce() {
			if (bouncy && !C2D::withinBounds(trans.position, board.asArea())) {
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
			} else if (loopy && shape && !C2D::withinBounds(board.asArea(), *shape)) {
				auto const
					min = board.min(),
					max = board.max()
				;
				if (trans.position.x < min.x) trans.position.x = max.x + shape->radius.max();
				if (trans.position.x > max.x) trans.position.x = min.x - shape->radius.max();
				if (trans.position.y < min.y) trans.position.y = max.y - shape->radius.max();
				if (trans.position.y > max.y) trans.position.y = min.y + shape->radius.max();
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

		void updateSprite(SpriteHandle const& sprite, bool glowSprite = false) {
			if (!sprite) return;
			sprite->visible = true;
			sprite->frame	= this->sprite.frame;
			sprite->size	= this->sprite.sheetSize;
			if (rotateSprite)
				sprite->local.rotation.z	= trans.rotation;
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
				shape->rotation	= trans.rotation;
			}
		}

		void animate() {
			switch (objectState) {
				case State::SOS_DESPAWNING: {
					if (counter++ < despawnTime) {
						spawnglow = counter / static_cast<float>(despawnTime);
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
					//	spawnsize = 1.0 + animColor.a * SPAWN_GOWTH;
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
						spawnglow = 1.0 - counter / static_cast<float>(despawnTime);
						animColor.a = counter / static_cast<float>(spawnTime);
						spawnsize = (1.0 + SPAWN_GOWTH) - animColor.a;
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

	struct BulletServerConfig: ServerConfig, ServerMeshConfig, ServerGlowMeshConfig, BoundedObjectConfig {
		ColliderConfig const colli = {
			Collision::Layer::ENEMY_BULLET,
			Collision::Tag::FOR_PLAYER_1
		};
		CollisionLayerConfig const layer = {
			Collision::Mask::ENEMY_BULLET,
			Collision::Mask::BULLET_ERASER,
		};
		BulletConfig::Collision const mask = {};
	};

	template<Type::Derived<Bullet> TBullet, Type::Derived<BulletConfig> TConfig>
	struct BulletServer<TBullet, TConfig>: AServer, AUpdateable {
		using CollisionMask = AGameObject::CollisionMask;

		using BulletType = TBullet;
		using ConfigType = TConfig;

		Graph::ReferenceHolder& mainMesh;
		Graph::ReferenceHolder& glowMesh;

		GameArea& board;
		GameArea& playfield;

		BulletServer(BulletServerConfig const& cfg):
			mainMesh(cfg.mainMesh),
			glowMesh(cfg.glowMesh),
			board(cfg.board),
			playfield(cfg.playfield) {
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
				all.back().mainSprite->local.position.z = zoff;
				if (&cfg.mainMesh != &cfg.glowMesh) {
					all.back().glowSprite = glowMesh.createReference<Graph::AnimatedPlaneRef>();
					all.back().glowSprite->local.position.z = zoff;
				}
				all.back().hideSprites();
				free.pushBack(&all.back());
			}
		}

		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Reference<BulletType> bullet = b.template as<BulletType>();
				bullet->clear();
				bullet->enable();
				return b;
			}
			return nullptr;
		}

		void onUpdate(float delta, Makai::App& app) override {
			for (auto& obj: used) {
				obj->onUpdate(delta);
			}
		}

		void discardAll() override {
			for (auto b: used) {
				BulletType& bullet = access<BulletType>(b);
				bullet.discard();
			};
		}
		
		void freeAll() override {
			for (auto b: used) {
				BulletType& bullet = access<BulletType>(b);
				bullet.free();
			};
		}

		void despawnAll() override {
			for (auto b: used) {
				BulletType& bullet = access<BulletType>(b);
				bullet.despawn();
			};
		}

		usize capacity() override {
			return all.size();
		}

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
		bool contains(HandleType const& object) override {
			return (used.find(object) != -1);
		}

		BulletServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			BulletType& bullet = *(object.template as<BulletType>());
			if (!bullet.isFree()) bullet.free();
			AServer::release(object);
			return *this;
		}

	private:
		StaticList<BulletType> all;
	};
}

#endif