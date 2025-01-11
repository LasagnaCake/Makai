#ifndef MAKAILIB_EX_GAME_DANMAKU_BULLET_H
#define MAKAILIB_EX_GAME_DANMAKU_BULLET_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct BulletServer;
	
	struct Bullet: ServerObject {
		Bullet(
			Server& server,
			Playfield& playfield,
			CollisionMask const& affects,
			CollisionMask const& affectedBy,
			CollisionMask const& tags
		): ServerObject(affects, affectedBy, tags), server(server), playfield(playfield) {
			collision()->shape = shape.as<C2D::IBound2D>();
		}

		Bullet& clear() override {
			ServerObject::clear();
			rotateSprite	= true;
			radius			= {};
			scale			= {};
			glowing			= false;
			return *this;
		}

		Bullet& reset() override {
			ServerObject::reset();
			radius.factor	= 0;
			scale.factor	= 0;
			return *this;
		}

		void onUpdate(float delta) override {
			if (objectState == State::AOS_FREE) return;
			ServerObject::onUpdate(delta);
			hideSprites();
			setSpriteVisibility(glowing || spawnglow, true);
			updateSprite(sprite.asWeak());
			updateSprite(glowSprite.asWeak());
			updateHitbox();
			animate();
			if (paused()) return;
			color.next();
			radius.next();
			trans.position	+= Math::angleV2(rotation.next()) * velocity.next() * delta;
			trans.rotation	= rotation.value;
			trans.scale		= scale.next();
			loopAndBounce();
			updateSprite(sprite.asWeak());
			updateSprite(glowSprite.asWeak());
			updateHitbox();
		}

		Bullet& discard(bool const force = false) override {
			if (discardable && !force) return *this;
			despawn();
		}

		bool isFree() const override {
			return false;
		}

		Bullet& spawn() override {
			counter = 0;
			objectState = State::AOS_SPAWNING;
			onAction(*this, Action::AOA_SPAWN_BEGIN);
		}

		Bullet& despawn() override {
			counter = 0;
			objectState = State::AOS_DESPAWNING;
			onAction(*this, Action::AOA_DESPAWN_BEGIN);
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (
				collider.tags.match(CollisionTag::PLAYER_GRAZEBOX).overlap()
			) setCollisionTags(getCollisionTags() & CollisionTag::GRAZEABLE.inverse());
			if (collider.tags.match(CollisionTag::BULLET_ERASER).overlap())
				discard();
		}

		Bullet& setSpriteFrame(Vector2 const& frame)	{if (sprite) sprite->frame = frame; return *this;	}
		Bullet& setSpriteSheetSize(Vector2 const& size)	{if (sprite) sprite->size = size; return *this;		}

		Bullet& setSprite(Vector2 const& sheetSize, Vector2 const& frame) {
			return setSpriteFrame(frame).setSpriteSheetSize(sheetSize);
		}

		Property<Vector2> radius;
		Property<Vector2> scale;

		bool rotateSprite = true;
		
		bool bouncy	= false;
		bool loopy	= false;

		usize spawnTime		= 5;
		usize despawnTime	= 5;

		bool glowing = false;

	private:
		Server&		server;
		Playfield&	playfield;

		SpriteInstance sprite		= nullptr;
		SpriteInstance glowSprite	= nullptr;

		usize counter	= 0;
		bool spawnglow	= false;

		Vector4 animColor = Graph::Color::WHITE;

		Instance<C2D::Circle> shape = new C2D::Circle(0);

		void loopAndBounce() {
			if (bouncy && !Collision::GJK::check(
				C2D::Box(playfield.center, playfield.size),
				C2D::Point(trans.position)
			)) {
				auto const
					tl = playfield.topLeft(),
					br = playfield.bottomRight()
				;
				if (trans.position.x < tl.x) shift(PI);
				if (trans.position.x > br.x) shift(PI);
				if (trans.position.y > tl.y) shift(0);
				if (trans.position.y < tl.y) shift(0);
				onAction(*this, Action::AOA_BOUNCE);
				bouncy = false;
			}
			else if (loopy && shape && !Collision::GJK::check(
				C2D::Box(playfield.center, playfield.size),
				*shape
			)) {
				auto const
					tl = playfield.topLeft(),
					br = playfield.bottomRight()
				;
				if (trans.position.x < tl.x) trans.position.x = br.x + shape->radius.x;
				if (trans.position.x > br.x) trans.position.x = tl.x - shape->radius.x;
				if (trans.position.y > tl.y) trans.position.y = br.y - shape->radius.y;
				if (trans.position.y < tl.y) trans.position.y = tl.y + shape->radius.y;
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
				server.release(this);
				active = false;
				hideSprites();
				objectState = State::AOS_FREE;
			} else {
				active = true;
				objectState = State::AOS_ACTIVE;
			}
		}

		friend class BulletServer;
	};

	struct BulletServerConfig {
		using CollisionMask = GameObject::CollisionMask;
		usize const				size;
		Graph::ReferenceHolder&	mainMesh;
		Graph::ReferenceHolder&	glowMesh;
		Playfield&				playfield;
		CollisionMask const		affects		= {};
		CollisionMask const		affectedBy	= {};
		CollisionMask const		tags		= {};
	};

	struct BulletServer: Server, IUpdateable {
		using CollisionMask = GameObject::CollisionMask;

		Graph::ReferenceHolder& mainMesh;
		Graph::ReferenceHolder& glowMesh;

		Playfield& playfield;

		BulletServer(BulletServerConfig const& cfg):
			mainMesh(cfg.mainMesh),
			glowMesh(cfg.glowMesh),
			playfield(cfg.playfield) {
			all.resize(cfg.size);
			free.resize(cfg.size);
			used.resize(cfg.size);
			for (usize i = 0; i < cfg.size; ++i) {
				all.pushBack(Bullet(*this, playfield, cfg.affects, cfg.affectedBy, cfg.tags));
				all.back().sprite = mainMesh.createReference<Graph::AnimatedPlaneRef>();
				if (&cfg.mainMesh != &cfg.glowMesh)
					all.back().glowSprite = glowMesh.createReference<Graph::AnimatedPlaneRef>();
				free.pushBack(&all.back());
			}
		}

		virtual HandleType acquire() override {
			Handle<Bullet> bullet = Server::acquire().as<Bullet>();
			bullet->clear();
			return bullet.as<GameObject>();
		}

		void onUpdate(float delta, App& app) override {
			for (auto& obj: used)
				obj->onUpdate(delta);
		}

	private:
		List<Bullet> all;
	};
}

#endif