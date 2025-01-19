#ifndef MAKAILIB_EX_GAME_DANMAKU_LASER_H
#define MAKAILIB_EX_GAME_DANMAKU_LASER_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct LaserServer;

	struct LaserConfig: ServerObjectConfig, GameObjectConfig {
		using GameObjectConfig::CollisionMask;
		struct Collision {
			CollisionMask const player = CollisionTag::FOR_PLAYER_1;
		} const mask;
	};

	struct Laser: AServerObject, ThreePatchContainer, AttackObject, Circular, Long, IToggleable {
		Laser(LaserConfig const& cfg):
			AServerObject(cfg), mask(mask), server(cfg.server) {
			collision()->shape = shape.as<C2D::IBound2D>();
		}

		Laser& clear() override {
			AServerObject::clear();
			radius			= {};
			velocity		= {};
			rotation		= {};
			length			= {};
			toggleState		= IToggleable::State::TS_UNTOGGLED;
			patch			= {};
			animColor		= Graph::Color::WHITE;
			return *this;
		}

		Laser& reset() override {
			if (isFree()) return *this;
			AServerObject::reset();
			velocity.factor	= 0;
			rotation.factor	= 0;
			radius.factor	= 0;
			scale.factor	= 0;
			length.factor	= 0;
			return *this;
		}

		Laser& toggle(bool const state, bool const immediately = false) override {
			if (isFree()) return *this;
			toggleCounter = 0;
			if (immediately) {
				toggleState = state ? IToggleable::State::TS_TOGGLED : IToggleable::State::TS_UNTOGGLED;
				toggleColor = state ? 1.0 : 0.5;
				return *this;
			}
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

		void onUpdate(float delta) override {
			sprite->visible = isFree();
			if (isFree()) return;
			AServerObject::onUpdate(delta);
			updateSprite();
			updateHitbox();
			animate();
			if (paused()) return;
			color.next();
			radius.next();
			length.next();
			damage.next();
			trans.position	+= Math::angleV2(rotation.next()) * velocity.next() * delta;
			trans.rotation	= rotation.value;
			trans.scale		= scale.next();
			animateToggle();
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
		}
		
		Laser& discard(bool const force = false) override {
			if (isFree()) return *this;
			if (discardable && !force) return *this;
			despawn();
			return *this;
		}

		Laser& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = AServerObject::State::SOS_SPAWNING;
			onAction(*this, Action::SOA_SPAWN_BEGIN);
			return *this;
		}

		Laser& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = AServerObject::State::SOS_DESPAWNING;
			onAction(*this, Action::SOA_DESPAWN_BEGIN);
			return *this;
		}

		Laser& setFree(bool const state) override {
			if (state) {
				active = false;
				sprite->visible = false;
				objectState = AServerObject::State::SOS_FREE;
				release(this, server);
			} else {
				active = true;
				objectState = AServerObject::State::SOS_ACTIVE;
			}
			return *this;
		}

		LaserConfig::Collision const mask;

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

		void updateSprite() {
			if (!sprite) return;
			sprite->local.rotation.z	= trans.rotation;
			sprite->local.position		= Vec3(trans.position, sprite->local.position.z);
			sprite->local.scale			= trans.scale;
			Vector4 const spriteColor = color.value * animColor;
			for (usize i: {0, 1, 2, 3}) {
				Vector2 const uvOffset = Vector2(i&1, (i&2)>>1);
				sprite->color.head[i] = spriteColor;
				sprite->color.body[i] = spriteColor;
				sprite->color.tail[i] = spriteColor;
				sprite->uv.head[i] = (patch.frame.head + uvOffset) / patch.size;
				sprite->uv.body[i] = (patch.frame.body + uvOffset) / patch.size;
				sprite->uv.tail[i] = (patch.frame.tail + uvOffset) / patch.size;
				if (patch.vertical) {
					sprite->uv.head[i] = sprite->uv.head[i].yx();
					sprite->uv.body[i] = sprite->uv.body[i].yx();
					sprite->uv.tail[i] = sprite->uv.tail[i].yx();
				}
			}
			sprite->size.head = radius.value.x;
			sprite->size.tail = radius.value.x;
			sprite->size.body = length.value;
		}

		void updateHitbox() {
			if (shape) {
				shape->width	= radius.value * trans.scale;
				shape->length	= length.value * trans.scale.x;
				shape->position	= trans.position;
				shape->rotation	= trans.rotation;
			}
		}

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
		}

		void animateToggle() {
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

	struct LaserServerConfig: ServerConfig, ServerMeshConfig, BoundedObjectConfig {
		ColliderConfig const colli = {
			CollisionLayer::ENEMY_LASER,
			{},
			CollisionTag::FOR_PLAYER_1
		};
		LaserConfig::Collision const mask = {};
	};

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
				float const zoff = i / static_cast<float>(cfg.size);
				all.pushBack(Laser({*this, cfg, cfg.colli, cfg.mask}));
				all.back().sprite = mainMesh.createReference<ThreePatchRef>();
				all.back().sprite->local.position.z = zoff;
				free.pushBack(&all.back());
			}
		}

		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Reference<Laser> laser = b.morph<Laser>();
				laser->setFree(false);
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
			Laser& laser = *object.morph<Laser>();
			AServer::release(object);
			return *this;
		}

	private:
		List<Laser> all;
	};
}

#endif