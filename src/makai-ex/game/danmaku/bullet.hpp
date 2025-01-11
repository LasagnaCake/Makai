#ifndef MAKAILIB_EX_GAME_DANMAKU_BULLET_H
#define MAKAILIB_EX_GAME_DANMAKU_BULLET_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct BulletServer;

	struct BulletConfig: ServerObjectConfig, GameObjectConfig {};
	
	struct Bullet: ServerObject {
		Bullet(BulletConfig const& cfg):
			ServerObject(cfg), server(cfg.server) {
			collision()->shape = shape.as<C2D::IBound2D>();
		}

		Bullet& clear() override {
			ServerObject::clear();
			rotateSprite	= true;
			radius			= {};
			scale			= {};
			velocity		= {};
			rotation		= {};
			dope			= false;
			glowing			= false;
			bouncy			= false;
			loopy			= false;
			return *this;
		}

		Bullet& reset() override {
			ServerObject::reset();
			velocity.factor	= 0;
			rotation.factor	= 0;
			radius.factor	= 0;
			scale.factor	= 0;
			return *this;
		}

		void onUpdate(float delta) override {
			if (isFree()) return;
			ServerObject::onUpdate(delta);
			hideSprites();
			setSpriteVisibility(glowing || spawnglow, true);
			updateSprite(sprite.asWeak());
			updateSprite(glowSprite.asWeak());
			updateHitbox();
			if (paused()) return;
			color.next();
			radius.next();
			trans.position	+= Math::angleV2(rotation.next()) * velocity.next() * delta;
			trans.rotation	= rotation.value;
			trans.scale		= scale.next();
			playfieldCheck();
			loopAndBounce();
			animate();
		}

		Bullet& discard(bool const force = false) override {
			if (isFree()) return *this;
			if (discardable && !force) return *this;
			despawn();
		}

		bool isFree() const override {
			return objectState == State::AOS_FREE;
		}

		Bullet& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = State::AOS_SPAWNING;
			onAction(*this, Action::AOA_SPAWN_BEGIN);
		}

		Bullet& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = State::AOS_DESPAWNING;
			onAction(*this, Action::AOA_DESPAWN_BEGIN);
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
			if (collider.tags.match(CollisionTag::BULLET_ERASER).overlap())
				discard();
		}

		Bullet& setSpriteFrame(Vector2 const& frame)	{
			if (!isFree() && sprite)
				sprite->frame = frame;
			return *this;
		}

		Bullet& setSpriteSheetSize(Vector2 const& size)	{
			if (!isFree() && sprite)
				sprite->size = size;
			return *this;	
		}

		Bullet& setSprite(Vector2 const& sheetSize, Vector2 const& frame) {
			if (isFree()) return *this;
			return setSpriteFrame(frame).setSpriteSheetSize(sheetSize);
		}

		Property<float>		velocity;
		Property<float>		rotation;
		Property<Vector2>	radius;

		bool dope = true;

		bool rotateSprite = true;
		
		bool bouncy	= false;
		bool loopy	= false;

		usize spawnTime		= 5;
		usize despawnTime	= 5;

		bool glowing = false;

	private:
		Server&		server;

		SpriteInstance sprite		= nullptr;
		SpriteInstance glowSprite	= nullptr;

		usize counter	= 0;
		bool spawnglow	= false;

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
			||	trans.position.y > tl.y
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
				onAction(*this, Action::AOA_BOUNCE);
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
				onAction(*this, Action::AOA_LOOP);
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
			else if (sprite)					sprite->visible		= state;
		}

		void hideSprites() {
			if (glowSprite)		glowSprite->visible	= false; 
			else if (sprite)	sprite->visible		= false;
		}

		void updateSprite(SpriteHandle const& sprite) {
			if (!sprite) return;
			if (rotateSprite)
				sprite->local.rotation	= trans.rotation;
			sprite->local.position		= trans.position;
			sprite->local.scale			= trans.scale;
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
				case State::AOS_DESPAWNING: {
					if (counter++ < despawnTime) {
						spawnglow = true;
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
					} else {
						spawnglow = false;
						onAction(*this, Action::AOA_DESPAWN_END);
						free();
					}
				}
				case State::AOS_SPAWNING: {
					if (counter++ < spawnTime) {
						spawnglow = true;
						animColor.a = counter / static_cast<float>(spawnTime);
					} else {
						spawnglow = false;
						setCollisionState(true);
						onAction(*this, Action::AOA_SPAWN_END);
						objectState = State::AOS_ACTIVE;
					}
				}
				[[likely]]
				default: break;
			}
		}

		friend class BulletServer;

		Bullet(Bullet const& other)	= default;
		Bullet(Bullet&& other)		= default;

		Bullet& setFree(bool const state) override {
			if (state) {
				active = false;
				hideSprites();
				objectState = State::AOS_FREE;
				release(this, server);
			} else {
				active = true;
				objectState = State::AOS_ACTIVE;
			}
			return *this;
		}

		friend class BulletServer;
	};

	struct BulletServerConfig: ServerConfig, ServerMeshConfig, GameObjectConfig {};

	struct BulletServer: Server, IUpdateable {
		using CollisionMask = GameObject::CollisionMask;

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
				all.pushBack(Bullet({*this, cfg}));
				all.back().sprite = mainMesh.createReference<Graph::AnimatedPlaneRef>();
				if (&cfg.mainMesh != &cfg.glowMesh)
					all.back().glowSprite = glowMesh.createReference<Graph::AnimatedPlaneRef>();
				free.pushBack(&all.back());
			}
		}

		virtual HandleType acquire() override {
			if (auto b = Server::acquire()) {
				Handle<Bullet> bullet = b.polymorph<Bullet>();
				bullet->clear();
				return bullet.as<GameObject>();
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

		HandleType acquire() override {
			auto b = Server::acquire();
			Bullet& bullet = *(b.as<Bullet>());
			bullet.clear();
			return b;
		}

	protected:
		BulletServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			Bullet& bullet = *(object.as<Bullet>());
			Server::release(object);
			return *this;
		}

	private:
		List<Bullet> all;
	};
}

#endif