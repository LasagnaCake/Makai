#ifndef MAKAILIB_EX_GAME_DANMAKU_BULLET_H
#define MAKAILIB_EX_GAME_DANMAKU_BULLET_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct BulletServer;

	struct BulletConfig: ServerObjectConfig, GameObjectConfig {
		using GameObjectConfig::CollisionMask;
		struct Collision {
			CollisionMask const eraser	= CollisionLayer::BULLET_ERASER;
			CollisionMask const player	= CollisionTag::FOR_PLAYER_1;
		} const colli = {};
	};
	
	struct Bullet: AServerObject, ISpriteContainer, AttackObject, Circular, Glowing, Dope, RotatesSprite {
		Bullet(BulletConfig const& cfg):
			AServerObject(cfg), server(cfg.server), colli(colli) {
			collision()->shape = shape.as<C2D::IBound2D>();
		}

		Bullet& clear() override {
			AServerObject::clear();
			rotateSprite	= true;
			radius			= {};
			scale			= {};
			velocity		= {};
			rotation		= {};
			sprite			= {};
			dope			= false;
			glowing			= false;
			bouncy			= false;
			loopy			= false;
			grazed			= false;
			animColor		= Graph::Color::WHITE;
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
			hideSprites();
			setSpriteVisibility(glowing || spawnglow, true);
			updateSprite(mainSprite.asWeak());
			updateSprite(glowSprite.asWeak());
			updateHitbox();
			animate();
			if (paused()) return;
			color.next();
			radius.next();
			//damage.next();
			trans.position	+= Math::angleV2(rotation.next()) * velocity.next() * delta;
			trans.rotation	= rotation.value;
			trans.scale		= scale.next();
			playfieldCheck();
			loopAndBounce();
		}

		Bullet& discard(bool const force = false) override {
			if (isFree()) return *this;
			if (discardable && !force) return *this;
			despawn();
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
				collider.affects.match(colli.eraser).overlap()
			&&	collider.tags.match(colli.player).overlap()
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
				hideSprites();
				objectState = State::SOS_FREE;
				release(this, server);
			} else {
				active = true;
				objectState = State::SOS_ACTIVE;
			}
			return *this;
		}
		
		bool bouncy	= false;
		bool loopy	= false;

		bool grazed	= false;

		BulletConfig::Collision const colli;

	private:
		AServer&	server;

		SpriteInstance mainSprite	= nullptr;
		SpriteInstance glowSprite	= nullptr;

		usize counter	= 0;
		bool spawnglow	= false;
		float spawnsize = 1;

		constexpr static float SPAWN_GOWTH = .5;

		Vector4 animColor = Graph::Color::WHITE;

		Instance<C2D::Circle> shape = new C2D::Circle(0);

		void playfieldCheck() {
			if (!dope) return;
			auto const
				tl = playfield.topLeft(),
				br = playfield.bottomRight()
			;
			if (
				trans.position.x < tl.x
			||	trans.position.x > br.x
			||	trans.position.y > br.y
			||	trans.position.y < tl.y
			) free();
		}

		void loopAndBounce() {
			if (bouncy && !Collision::GJK::check(
				board.asArea(),
				C2D::Point(trans.position)
			)) {
				auto const
					tl = board.topLeft(),
					br = board.bottomRight()
				;
				if (trans.position.x < tl.x) shift(PI);
				if (trans.position.x > br.x) shift(PI);
				if (trans.position.y > tl.y) shift(0);
				if (trans.position.y < tl.y) shift(0);
				onAction(*this, Action::SOA_BOUNCE);
				bouncy = false;
			} else if (loopy && shape && !Collision::GJK::check(
				board.asArea(),
				*shape
			)) {
				auto const
					tl = board.topLeft(),
					br = board.bottomRight()
				;
				if (trans.position.x < tl.x) trans.position.x = br.x + shape->radius.max();
				if (trans.position.x > br.x) trans.position.x = tl.x - shape->radius.max();
				if (trans.position.y > tl.y) trans.position.y = br.y - shape->radius.max();
				if (trans.position.y < tl.y) trans.position.y = tl.y + shape->radius.max();
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

		void updateSprite(SpriteHandle const& sprite) {
			if (!sprite) return;
			sprite->frame	= this->sprite.frame;
			sprite->size	= this->sprite.sheetSize;
			if (rotateSprite)
				sprite->local.rotation.z	= trans.rotation;
			sprite->local.position			= Vec3(trans.position, sprite->local.position.z);
			sprite->local.scale				= trans.scale;
			sprite->setColor(animColor * color.value);
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
						spawnglow = true;
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
					//	spawnsize = 1.0 + animColor.a * SPAWN_GOWTH;
					} else {
						spawnglow = false;
						animColor.a = 0;
						spawnsize = 1.0;
						onAction(*this, Action::SOA_DESPAWN_END);
						free();
					}
				} break;
				case State::SOS_SPAWNING: {
					if (counter++ < spawnTime) {
						spawnglow = true;
						animColor.a = counter / static_cast<float>(spawnTime);
						spawnsize = (1.0 + SPAWN_GOWTH) - animColor.a;
					} else {
						spawnglow = false;
						animColor.a = 1;
						spawnsize = 1.0;
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

		friend class BulletServer;
	};

	struct BulletServerConfig: ServerConfig, ServerMeshConfig, ServerGlowMeshConfig, BoundedObjectConfig {
		ColliderConfig const colli = {
			CollisionLayer::ENEMY_BULLET,
			CollisionLayer::BULLET_ERASER,
			CollisionTag::FOR_PLAYER_1
		};
		BulletConfig::Collision const mask = {};
	};

	struct BulletServer: AServer, AUpdateable {
		using CollisionMask = AGameObject::CollisionMask;

		Graph::ReferenceHolder& mainMesh;
		Graph::ReferenceHolder& glowMesh;

		GameArea& board;
		GameArea& playfield;

		BulletServer(BulletServerConfig const& cfg):
			mainMesh(cfg.mainMesh),
			glowMesh(cfg.glowMesh),
			board(cfg.board),
			playfield(cfg.playfield) {
			all.resize(cfg.size);
			free.resize(cfg.size);
			used.resize(cfg.size);
			for (usize i = 0; i < cfg.size; ++i) {
				float const zoff = i / static_cast<float>(cfg.size);
				all.pushBack(Bullet({*this, cfg, cfg.colli, cfg.mask}));
				all.back().mainSprite = mainMesh.createReference<Graph::AnimatedPlaneRef>();
				all.back().mainSprite->local.position.z = zoff;
				if (&cfg.mainMesh != &cfg.glowMesh) {
					all.back().glowSprite = glowMesh.createReference<Graph::AnimatedPlaneRef>();
					all.back().glowSprite->local.position.z = zoff;
				}
				free.pushBack(&all.back());
			}
		}

		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Reference<Bullet> bullet = b.morph<Bullet>();
				bullet->setFree(false);
				bullet->clear();
				return bullet.as<AGameObject>();
			}
			return nullptr;
		}

		void onUpdate(float delta, App& app) override {
			for (auto& obj: used) {
				obj->onUpdate(delta);
			}
		}

		void discardAll() override {
			for (auto b: used) {
				Bullet& bullet = access<Bullet>(b);
				bullet.discard();
			};
		}
		
		void freeAll() override {
			for (auto b: used) {
				Bullet& bullet = access<Bullet>(b);
				bullet.free();
			};
		}

		void despawnAll() override {
			for (auto b: used) {
				Bullet& bullet = access<Bullet>(b);
				bullet.despawn();
			};
		}

		usize capacity() override {
			return all.size();
		}

		ObjectQueryType getInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				Bullet& bullet = access<Bullet>(b);
				if (
					bullet.shape
				&&	Collision::GJK::check(*bullet.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

		ObjectQueryType getNotInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				Bullet& bullet = access<Bullet>(b);
				if (
					bullet.shape
				&&	!Collision::GJK::check(*bullet.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

	protected:
		BulletServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			Bullet& bullet = *(object.as<Bullet>());
			AServer::release(object);
			return *this;
		}

	private:
		List<Bullet> all;
	};
}

#endif