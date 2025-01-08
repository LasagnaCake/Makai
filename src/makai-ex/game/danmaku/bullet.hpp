#ifndef MAKAILIB_EX_GAME_DANMAKU_BULLET_H
#define MAKAILIB_EX_GAME_DANMAKU_BULLET_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct BulletServer;
	
	struct Bullet: AttackObject {
		Bullet(Server& server): server(server) {}

		bool rotateSprite = false;

		Bullet& clear() override {
			trans			= Transform2D();
			velocity		= {};
			rotation		= {};
			dope			= false;
			discardable		= true;
			rotateSprite	= false;
			affects			= {};
			affectedBy		= {};
			shape			= nullptr;
			canCollide		= true;
			task			= doNothing();
			pause			= {};
			onAction.clear();
			onObjectUpdate.clear();
			return *this;
		}

		Bullet& reset() override {
			velocity.current	= velocity.from;
			rotation.current	= rotation.from;
			velocity.factor		= 0;
			rotation.factor		= 0;
			return *this;
		}

		void onUpdate(float delta, App& app) override {
			AttackObject::onUpdate(delta, app);
		}

		Bullet& discard(bool const force = false) override {
			if (discardable && !force) return *this;
		}

		bool isFree() const override {
			return false;
		}

		Bullet& spawn() override {
		}

		Bullet& despawn() override {
			free();
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {

		}
		
	private:
		friend class BulletServer;

		Bullet(Bullet const& other) = default;

		Instance<Graph::AnimatedPlaneRef> sprite = nullptr;

		Bullet& setFree(bool const state) override {
			if (state) {
				server.release(this);
				active = false;
			}
		}

		Server& server;
		friend class BulletServer;
	};

	struct BulletServer: Server {
		BulletServer(usize const size) {
			all.resize(size, Bullet(*this));
			free.resize(size);
			used.resize(size);
			for (Bullet& bullet: all)
				free.pushBack(&bullet);
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