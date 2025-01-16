#ifndef MAKAILIB_EX_GAME_DANMAKU_LASER_H
#define MAKAILIB_EX_GAME_DANMAKU_LASER_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct LaserServer;

	struct LaserConfig: ServerObjectConfig, GameObjectConfig {};

	struct Laser: AServerObject, IThreePatchContainer, AttackObject, Circular, Long, IToggleable {
		Laser(LaserConfig const& cfg):
			AServerObject(cfg), server(cfg.server) {
			collision()->shape = shape.as<C2D::IBound2D>();
		}

		Laser& clear() override {
			AServerObject::clear();
			radius		= {};
			scale		= {};
			velocity	= {};
			rotation	= {};
			length		= {};
			toggleState	= IToggleable::State::TS_UNTOGGLED;
			toggleCounter = 0;
			return *this;
		}

		Laser& reset() override {
			AServerObject::reset();
			velocity.factor	= 0;
			rotation.factor	= 0;
			radius.factor	= 0;
			scale.factor	= 0;
			length.factor	= 0;
			return *this;
		}

		Laser& toggle(bool const state) override {
			toggleCounter = 0;
			nextState = state ? IToggleable::State::TS_TOGGLED : IToggleable::State::TS_UNTOGGLED;
			if (toggleState != nextState) {
				switch (toggleState) {
					case IToggleable::State::TS_TOGGLED:
					case IToggleable::State::TS_TOGGLING:
						toggleState = IToggleable::State::TS_UNTOGGLING;
						break;
					case IToggleable::State::TS_UNTOGGLED:
					case IToggleable::State::TS_UNTOGGLING:
						toggleState = IToggleable::State::TS_TOGGLING;
						break;
				}
			}
			return *this;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
			/*
			if (collider.tags.match(CollisionTag::BULLET_ERASER).overlap())
				discard();
			*/
		}

		
		Laser& discard(bool const force = false) override {
			if (isFree()) return *this;
			if (discardable && !force) return *this;
			despawn();
		}

		Laser& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = AServerObject::State::SOS_SPAWNING;
			onAction(*this, Action::SOA_SPAWN_BEGIN);
		}

		Laser& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = AServerObject::State::SOS_DESPAWNING;
			onAction(*this, Action::SOA_DESPAWN_BEGIN);
		}

	private:
		AServer&	server;

		usize toggleCounter	= 0;
		usize counter		= 0;

		IToggleable::State nextState = IToggleable::State::TS_TOGGLED;

		ThreePatchInstance sprite	= nullptr;

		Vector4 animColor = Graph::Color::WHITE;

		float toggleColor = 1.0f;

		Instance<C2D::Capsule> shape = new C2D::Capsule(0);

		friend class LaserServer;

		Laser(Laser const& other)	= default;
		Laser(Laser&& other)		= default;

		void animate() {
			switch (objectState) {
				case AServerObject::State::SOS_DESPAWNING: {
					if (counter++ < despawnTime) {
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime) * toggleColor;
					} else {
						onAction(*this, Action::SOA_DESPAWN_END);
						free();
					}
				} break;
				case AServerObject::State::SOS_SPAWNING: {
					if (counter++ < spawnTime) {
						animColor.a = counter / static_cast<float>(spawnTime) * toggleColor;
					} else {
						setCollisionState(true);
						onAction(*this, Action::SOA_SPAWN_END);
						objectState = AServerObject::State::SOS_ACTIVE;
					}
				} break;
				[[likely]]
				default: break;
			}
			switch (toggleState) {
				case IToggleable::State::TS_UNTOGGLING: {
					if (toggleCounter++ < untoggleTime) {
						toggleColor = 0.5 * (2.0 - (toggleCounter / static_cast<float>(untoggleTime)));
					} else {
						toggleColor = 0.5;
						toggleState = IToggleable::State::TS_UNTOGGLED;
					}
				} break;
				case IToggleable::State::TS_TOGGLING: {
					if (toggleCounter++ < toggleTime) {
						toggleColor = 0.5 * (1.0 + (toggleCounter / static_cast<float>(toggleTime)));
					} else {
						toggleColor = 1.0;
						toggleState = IToggleable::State::TS_TOGGLED;
					}
				} break;
				[[likely]]
				default: break;
			}
		}
	};

	struct LaserServerConfig: ServerConfig, ServerMeshConfig, GameObjectConfig {};

	struct LaserServer: AServer, AUpdateable {

		Graph::ReferenceHolder& mainMesh;

		GameArea& board;
		GameArea& playfield;

		LaserServer(LaserServerConfig const& cfg):
			mainMesh(cfg.mainMesh),
			board(cfg.board),
			playfield(cfg.playfield) {
			all.resize(cfg.size);
			free.resize(cfg.size);
			used.resize(cfg.size);
			for (usize i = 0; i < cfg.size; ++i) {
				all.pushBack(Laser({*this, cfg}));
				all.back().sprite = mainMesh.createReference<ThreePatchRef>();
				free.pushBack(&all.back());
			}
		}

		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Handle<Laser> laser = b.polymorph<Laser>();
				laser->clear();
				laser->setFree(false);
				return laser.as<AGameObject>();
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
				Laser& laser = access<Laser>(b);
				laser.discard();
			};
		}
		
		void freeAll() override {
			for (auto b: used) {
				Laser& laser = access<Laser>(b);
				laser.free();
			};
		}

		void despawnAll() override {
			for (auto b: used) {
				Laser& laser = access<Laser>(b);
				laser.despawn();
			};
		}

		usize capacity() override {
			return all.size();
		}

		ObjectQueryType getInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				Laser& laser = access<Laser>(b);
				if (
					laser.shape
				&&	Collision::GJK::check(*laser.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

		ObjectQueryType getNotInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				Laser& laser = access<Laser>(b);
				if (
					laser.shape
				&&	!Collision::GJK::check(*laser.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

	protected:
		LaserServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			Laser& laser = *object.polymorph<Laser>();
			AServer::release(object);
			return *this;
		}

	private:
		List<Laser> all;
	};
}

#endif