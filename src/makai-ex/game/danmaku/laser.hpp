#ifndef MAKAILIB_EX_GAME_DANMAKU_LASER_H
#define MAKAILIB_EX_GAME_DANMAKU_LASER_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct Laser;
	struct LaserConfig;

	template<class T = Laser, class C = LaserConfig> struct LaserServer;

	struct LaserConfig: ServerObjectConfig, GameObjectConfig {
		using GameObjectConfig::CollisionMask;
		struct Collision {
			CollisionMask const player = Danmaku::Collision::Tag::FOR_PLAYER_1;
		} const mask;
	};

	struct Laser: AServerObject, ThreePatchContainer, AttackObject, Circular, Long, IToggleable {
		Laser(LaserConfig const& cfg):
			AServerObject(cfg), mask(cfg.mask), server(cfg.server) {
			collision()->shape = shape.template as<C2D::IBound2D>();
		}

		virtual ~Laser() {}

		Laser& clear() override {
			AServerObject::clear();
			radius			= {1};
			velocity		= {};
			rotation		= {};
			length			= {1};
			damage			= {};
			patch			= {};
			autoDecay		= false;
			fakeOut			= false;
			toggleState		= IToggleable::State::TS_UNTOGGLED;
			animColor		= Graph::Color::WHITE;
			counter			= 0;
			toggleCounter	= 0;
			toggleColor		= 0.5;
			toggleTime		= 5;
			untoggleTime	= 5;
			setCollisionState(false);
			initSprite();
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
				collision()->canCollide = state;
				return *this;
			}
			toggleColor = state ? 0.5 : 1.0;
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
			sprite->visible = !isFree();
			if (isFree()) return;
			AServerObject::onUpdate(delta);
			updateSprite();
			updateHitbox();
			animate();
			if (paused()) return;
			color.next();
			radius.next();
			length.next();
			if (autoDecay) damage.next();
			trans.position	+= Math::angleV2(rotation.next()) * velocity.next() * delta;
			trans.rotation	= rotation.value;
			trans.scale		= scale.next();
			animateToggle();
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
		}
		
		Laser& discard(bool const immediately = false, bool const force = false) override {
			if (isFree()) return *this;
			if (!discardable && !force) return *this;
			if (!immediately)	despawn();
			else				free();
			return *this;
		}

		Laser& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			animColor.a = 0;
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
			active = state;
			sprite->visible = !state;
			if (state) {
				objectState = AServerObject::State::SOS_FREE;
				clear();
				release(this, server);
			} else {
				objectState = AServerObject::State::SOS_ACTIVE;
			}
			return *this;
		}

		bool fakeOut = false;

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

		template <class, class> friend class LaserServer;

		Laser(Laser const& other)	= default;
		Laser(Laser&& other)		= default;

		void initSprite() {
			if (sprite) sprite->local.scale = 0;
		}

		void updateSprite() {
			if (!sprite) return;
			sprite->local.rotation.z	= trans.rotation;
			sprite->local.position		= Vec3(trans.position, sprite->local.position.z);
			sprite->local.scale			= trans.scale;
			Vector4 const spriteColor = color.value * animColor * (fakeOut ? 1 : Graph::Color::alpha(toggleColor));
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
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
					} else {
						counter = 0;
						onAction(*this, Action::SOA_DESPAWN_END);
						free();
					}
				} break;
				case AServerObject::State::SOS_SPAWNING: {
					if (counter++ < spawnTime) {
						animColor.a = counter / static_cast<float>(spawnTime);
					} else {
						counter = 0;
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
						toggleCounter = 0;
						toggleColor = 0.5;
						toggleState = IToggleable::State::TS_UNTOGGLED;
						setCollisionState(false);
					}
				} break;
				case IToggleable::State::TS_TOGGLING: {
					if (toggleCounter++ < toggleTime) {
						toggleColor = 0.5 * (1.0 + (toggleCounter / static_cast<float>(toggleTime)));
					} else {
						toggleCounter = 0;
						toggleColor = 1.0;
						toggleState = IToggleable::State::TS_TOGGLED;
						setCollisionState(true);
					}
				} break;
				[[likely]]
				default: break;
			}
		}
	};

	struct LaserCollisionConfig: CollisionObjectConfig<
		ColliderConfig{
			Danmaku::Collision::Layer::ENEMY_LASER,
			Danmaku::Collision::Tag::FOR_PLAYER_1
		},
		CollisionLayerConfig{
			Danmaku::Collision::Mask::ENEMY_LASER,
			{}
		},
		LaserConfig::Collision{}
	> {
		using CollisionObjectConfig::CollisionObjectConfig;
	};

	struct LaserServerConfig:
		ServerConfig,
		ServerMeshConfig,
		BoundedObjectConfig,
		LaserCollisionConfig {};

	struct LaserServerInstanceConfig: ServerConfig, LaserCollisionConfig {};

	template<Type::Derived<Laser> TLaser, Type::Derived<LaserConfig> TConfig>
	struct LaserServer<TLaser, TConfig>: AServer, AUpdateable, ReferencesSpriteMesh, ReferencesGameBounds {
		using LaserType		= TLaser;
		using ConfigType	= TConfig;

		LaserServer(LaserServerConfig const& cfg):
			ReferencesSpriteMesh{cfg.mainMesh},
			ReferencesGameBounds{cfg.board, cfg.playfield} {
			auto& cl		= CollisionServer::layers[cfg.colli.layer];
			cl.affects		= cfg.layer.affects;
			cl.affectedBy	= cfg.layer.affectedBy;
			all.resize(cfg.size);
			free.resize(cfg.size);
			used.resize(cfg.size);
			for (usize i = 0; i < cfg.size; ++i) {
				float const zoff = i / static_cast<float>(cfg.size);
				all.constructBack(ConfigType{*this, cfg, cfg.colli, cfg.mask});
				all.back().sprite = mainMesh.createReference<ThreePatchRef>();
				all.back().sprite->local.position.z = -zoff;
				all.back().sprite->visible = false;
				all.back().setCollisionState(false);
				free.pushBack(&all.back());
			}
		}

		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Reference<LaserType> laser = b.template as<LaserType>();
				laser->clear();
				laser->enable();
				return b;
			}
			return nullptr;
		}

		void onUpdate(float delta, Makai::App& app) override {
			if (used.empty()) return;
			for (auto& obj: all)
				if (!obj.isFree())
					obj.onUpdate(delta);
		}

		void discardAll() override {
			for (auto b: used) {
				LaserType& laser = access<LaserType>(b);
				laser.discard();
			};
		}
		
		void freeAll() override {
			for (auto b: used) {
				LaserType& laser = access<LaserType>(b);
				laser.free();
			};
		}

		void despawnAll() override {
			for (auto b: used) {
				LaserType& laser = access<LaserType>(b);
				laser.despawn();
			};
		}

		usize capacity() override {
			return all.size();
		}

		ObjectQueryType getInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				LaserType& laser = access<LaserType>(b);
				if (
					laser.shape
				&&	C2D::withinBounds(*laser.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

		ObjectQueryType getNotInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				LaserType& laser = access<LaserType>(b);
				if (
					laser.shape
				&&	!C2D::withinBounds(*laser.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

	protected:
		bool contains(HandleType const& object) override {
			return (used.find(object) != -1);
		}
		
		LaserServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			LaserType& laser = *object.template as<LaserType>();
			if (!laser.isFree()) laser.free();
			AServer::release(object);
			return *this;
		}

	private:
		StaticList<Laser> all;
	};
}

#endif