#ifndef MAKAILIB_EX_GAME_DANMAKU_LASER_H
#define MAKAILIB_EX_GAME_DANMAKU_LASER_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct LaserServer;

	struct LaserConfig: ServerObjectConfig, GameObjectConfig {};

	struct Laser: AServerObject, IThreePatchContainer, AttackObject, Circular, IToggleable {
		Laser(LaserConfig const& cfg):
			AServerObject(cfg), server(cfg.server) {
			collision()->shape = shape.as<C2D::IBound2D>();
		}

	private:
		AServer&	server;

		usize counter	= 0;
		bool spawnglow	= false;

		ThreePatchInstance sprite	= nullptr;

		Vector4 animColor = Graph::Color::WHITE;

		Instance<C2D::Capsule> shape = new C2D::Capsule(0);

		friend class LaserServer;

		Laser(Laser const& other)	= default;
		Laser(Laser&& other)		= default;

		void animate() {
			switch (objectState) {
				case AServerObject::State::SOS_DESPAWNING: {
					if (counter++ < despawnTime) {
						spawnglow = true;
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
					} else {
						spawnglow = false;
						onAction(*this, Action::SOA_DESPAWN_END);
						free();
					}
				}
				case AServerObject::State::SOS_SPAWNING: {
					if (counter++ < spawnTime) {
						spawnglow = true;
						animColor.a = counter / static_cast<float>(spawnTime);
					} else {
						spawnglow = false;
						setCollisionState(true);
						onAction(*this, Action::SOA_SPAWN_END);
						objectState = AServerObject::State::SOS_ACTIVE;
					}
				}
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