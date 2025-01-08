#ifndef MAKAILIB_EX_GAME_DANMAKU_BULLET_H
#define MAKAILIB_EX_GAME_DANMAKU_BULLET_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct BulletServer;
	
	struct Bullet: AttackObject {
		Bullet(
			Server& server,
			CollisionMask const& affects,
			CollisionMask const& affectedBy
		): AttackObject(affects, affectedBy), server(server) {
			collision()->shape = shape.as<C2D::IBound2D>();
		}

		Property<Vector2> radius;
		Property<Vector2> scale;

		bool rotateSprite = true;

		Bullet& clear() override {
			AttackObject::clear();
			rotateSprite	= true;
			radius			= {};
			scale			= {};
			return *this;
		}

		Bullet& reset() override {
			AttackObject::reset();
			radius.factor	= 0;
			scale.factor	= 0;
			return *this;
		}

		void onUpdate(float delta) override {
			AttackObject::onUpdate(delta);
			updateSprite(sprite.asWeak());
			updateSprite(glowSprite.asWeak());
			animate();
			if (paused()) return;
			trans.position	+= Math::angleV2(rotation.next()) * velocity.next() * delta;
			trans.rotation	= rotation.value;
			trans.scale		= scale.next();
			if (shape) {
				shape->radius	= radius.next() * trans.scale;
				shape->position	= trans.position;
				shape->rotation	= trans.rotation;
			}
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
		}

		Bullet& despawn() override {
			counter = 0;
			objectState = State::AOS_DESPAWNING;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (collider.tags.match(CollisionTag::BULLET_ERASER).overlap()) {
				discard();
			}
		}

		usize spawnTime		= 5;
		usize despawnTime	= 5;

		Bullet& setSpriteFrame(Vector2 const& frame)	{if (sprite) sprite->frame = frame; return *this;	}
		Bullet& setSpriteSheetSize(Vector2 const& size)	{if (sprite) sprite->size = size; return *this;		}

		Bullet& setSprite(Vector2 const& sheetSize, Vector2 const& frame) {
			return setSpriteFrame(frame).setSpriteSheetSize(sheetSize);
		}

	private:
		SpriteInstance sprite		= nullptr;
		SpriteInstance glowSprite	= nullptr;

		usize counter = 0;

		Vector4 animColor = Graph::Color::WHITE;

		Instance<C2D::Circle> shape = new C2D::Circle(0);

		void updateSprite(SpriteHandle const& sprite) {
			if (!sprite) return;
			if (rotateSprite)
				sprite->local.rotation	= trans.rotation;
			sprite->local.position		= trans.position;
			sprite->local.scale			= trans.scale;
			sprite->setColor(animColor * color.next());
		}

		void animate() {
			switch (objectState) {
				case State::AOS_DESPAWNING: {
					if (++counter < despawnTime) {
						animColor.a = 1.0 - counter / float(despawnTime);
					} else free();
				}
				case State::AOS_SPAWNING: {
					if (++counter < spawnTime) {
						animColor.a = counter / float(spawnTime);
					} else objectState = State::AOS_ACTIVE;
				}
				[[likely]]
				default: break;
			}
		}

		friend class BulletServer;

		Bullet(Bullet const& other)	= default;
		Bullet(Bullet&& other)		= default;

		Instance<Graph::AnimatedPlaneRef> sprite = nullptr;

		Bullet& setFree(bool const state) override {
			if (state) {
				server.release(this);
				active = false;
				if (sprite) {
					sprite->visible = false;
				}
			}
		}

		Server& server;
		friend class BulletServer;
	};

	struct BulletServer: Server, IUpdateable {
		using CollisionMask = GameObject::CollisionMask;

		BulletServer(
			usize const size,
			CollisionMask const& affects = {},
			CollisionMask const& affectedBy = {}
		) {
			all.resize(size);
			free.resize(size);
			used.resize(size);
			for (usize i = 0; i < size; ++i) {
				all.pushBack(Bullet(*this, affects, affectedBy));
				free.pushBack(&all.back());
			}
				
		}

		virtual HandleType acquire() override {
			Handle<Bullet> bullet = Server::acquire().as<Bullet>();
			bullet->clear();
			return bullet.as<GameObject>();
		}

	private:
		List<Bullet> all;
	};
}

#endif