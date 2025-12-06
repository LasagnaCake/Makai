#ifndef MAKAILIB_EX_GAME_DANMAKU_LASER_H
#define MAKAILIB_EX_GAME_DANMAKU_LASER_H

#include "core.hpp"
#include "makai/graph/gl/renderer/reference/patch.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	/// @brief Laser server laser.
	struct Laser;
	/// @brief Laser configuration.
	struct LaserConfig;

	/// @brief Laser server.
	/// @tparam T Laser type. By default, it is `Laser`.
	/// @tparam C Laser configuration type. By default, it is `LaserConfig`.
	template<class T = Laser, class C = LaserConfig> struct LaserServer;

	/// @brief Laser configuration.
	struct LaserConfig: ServerObjectConfig, GameObjectConfig {
		using GameObjectConfig::CollisionMask;
		struct Collision {
			CollisionMask const player = Danmaku::Collision::Tag::FOR_PLAYER_1;
		} const mask;
	};
	
	/// @brief Laser server laser.
	struct Laser: AServerObject, ThreePatchContainer, AttackObject, Circular, Long, IToggleable {
		/// @brief Constructs the laser.
		/// @param cfg Laser configuration to use.
		Laser(LaserConfig const& cfg):
			AServerObject(cfg), mask(cfg.mask), server(cfg.server) {
			collision()->shape = shape.template as<C2D::IBound2D>();
		}

		/// @brief Destructor.
		virtual ~Laser() {}

		/// @brief Resets all of the object's properties to their default values.
		/// @return Reference to self.
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

		/// @brief Restarts the object's transformable properties to the beginning.
		/// @return Reference to self.
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

		/// @brief Sets the object's toggle state.
		/// @param state Wether to toggle or untoggle.
		/// @param immediately Whether to toggle immediately. By default, it is false.
		/// @return Reference to self.
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

		/// @brief Executes every update cycle.
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

		/// @brief Executes when a collision event happens.
		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
		}
		
		/// @brief Discards the object, if applicable.
		/// @param immediately Whether to despawn first, or discard directly. By default, it is `false`.
		/// @param force Whether to force discard. By default, it is `false`.
		/// @return Reference to self.
		Laser& discard(bool const immediately = false, bool const force = false) override {
			if (isFree()) return *this;
			if (!discardable && !force) return *this;
			if (!immediately)	despawn();
			else				free();
			return *this;
		}

		/// @brief Spawns the object.
		/// @return Reference to self.
		Laser& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			animColor.a = 0;
			objectState = AServerObject::State::SOS_SPAWNING;
			onAction(*this, Action::SOA_SPAWN_BEGIN);
			return *this;
		}

		/// @brief Despawns the object.
		/// @return Reference to self.
		Laser& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = AServerObject::State::SOS_DESPAWNING;
			onAction(*this, Action::SOA_DESPAWN_BEGIN);
			return *this;
		}

		/// @brief Sets the object's "free state".
		/// @param state Whether to set the object as free or as active.
		/// @return Reference to self.
		Laser& setFree(bool const state) override {
			setCollisionState(false);
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

		/// @brief Whether to fake being toggled when untoggled.
		bool fakeOut = false;
		
		/// @brief Collision mask associated with the laser.
		LaserConfig::Collision const mask;

	private:
		/// @brief Server associated with the object.
		AServer&	server;

		/// @brief Counter used for togge/untoggle timing purposes.
		usize toggleCounter	= 0;
		/// @brief Counter used for spawn/despawn timing purposes.
		usize counter		= 0;
		
		/// @brief Next toggle state.
		IToggleable::State nextState = IToggleable::State::TS_TOGGLED;

		/// @brief Laser sprite.
		ThreePatchInstance sprite	= nullptr;

		/// @brief Current animation color.
		Vector4 animColor = Graph::Color::WHITE;

		/// @brief Current toggle color factor.
		float toggleColor = 1.0f;

		/// @brief Collision shape.
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
			for (usize i: {0, 2, 4}) {
				sprite->shape.colors[i][0]		= spriteColor;
				sprite->shape.colors[i][1]		= spriteColor;
				sprite->shape.colors[i+1][0]	= spriteColor;
				sprite->shape.colors[i+1][1]	= spriteColor;
				sprite->shape.uvs[i][0]		= (patch.frame.head.toVector2() + Vector2(0, 0)) / patch.size.toVector2();
				sprite->shape.uvs[i][1]		= (patch.frame.body.toVector2() + Vector2(0, 1)) / patch.size.toVector2();
				sprite->shape.uvs[i+1][0]	= (patch.frame.head.toVector2() + Vector2(1, 0)) / patch.size.toVector2();
				sprite->shape.uvs[i+1][1]	= (patch.frame.body.toVector2() + Vector2(1, 1)) / patch.size.toVector2();
				if (patch.vertical) {
					sprite->shape.uvs[i][0]		= sprite->shape.uvs[i][0].yx();
					sprite->shape.uvs[i][1]		= sprite->shape.uvs[i][1].yx();
					sprite->shape.uvs[i+1][0]	= sprite->shape.uvs[i+1][0].yx();
					sprite->shape.uvs[i+1][1]	= sprite->shape.uvs[i+1][1].yx();
				}
			}
			sprite->shape.sizes[0]	= radius.value.x;
			sprite->shape.sizes[1]	= length.value;
			sprite->shape.sizes[2]	= radius.value.x;
			sprite->shape.height	= radius.value.y;
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
	
	/// @brief Laser collision configuration.
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

	/// @brief Laser server configuration.
	struct LaserServerConfig:
		ServerConfig,
		ServerMeshConfig,
		BoundedObjectConfig,
		LaserCollisionConfig {};

	/// @brief Laser server instance configuration.
	struct LaserServerInstanceConfig: ServerConfig, LaserCollisionConfig {};

	/// @brief Laser server.
	/// @tparam T Laser type. By default, it is `Laser`.
	/// @tparam C Laser configuration type. By default, it is `LaserConfig`.
	template<Type::Derived<Laser> TLaser, Type::Derived<LaserConfig> TConfig>
	struct LaserServer<TLaser, TConfig>: AServer, AUpdateable, ReferencesSpriteMesh, ReferencesGameBounds {
		/// @brief Laser type.
		using LaserType		= TLaser;
		/// @brief Laser configuration type.
		using ConfigType	= TConfig;

		/// @brief Constructs the laser server.
		/// @param cfg Laser server configuration.
		LaserServer(LaserServerConfig const& cfg):
			ReferencesSpriteMesh{cfg},
			ReferencesGameBounds{cfg} {
			auto& cl		= CollisionServer::layers[cfg.colli.layer];
			cl.affects		= cfg.layer.affects;
			cl.affectedBy	= cfg.layer.affectedBy;
			all.resize(cfg.capacity);
			free.resize(cfg.capacity);
			used.resize(cfg.capacity);
			for (usize i = 0; i < cfg.capacity; ++i) {
				float const zoff = i / static_cast<float>(cfg.capacity);
				all.constructBack(ConfigType{*this, cfg, cfg.colli, cfg.mask});
				all.back().sprite = mainMesh.createReference<Graph::Ref::ThreePatch1D>();
				all.back().sprite->local.position.z = -zoff;
				all.back().sprite->visible = false;
				all.back().setCollisionState(false);
				free.pushBack(&all.back());
			}
		}

		/// @brief Tries to acquire a laser.
		/// @return Reference to laser, or `nullptr`.
		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Reference<LaserType> laser = b.template as<LaserType>();
				laser->clear();
				laser->enable();
				return b;
			}
			return nullptr;
		}

		/// @brief Executed every update cycle.
		void onUpdate(float delta, Makai::App& app) override {
			if (used.empty()) return;
			for (auto& obj: all)
				if (!obj.isFree())
					obj.onUpdate(delta);
		}

		/// @brief Discards all active items, if applicable.
		void discardAll() override {
			auto const uc = used;
			for (auto b: uc) {
				LaserType& laser = access<LaserType>(b);
				laser.discard();
			};
		}
		
		/// @brief Frees all active lasers.
		void freeAll() override {
			auto const uc = used;
			for (auto b: uc) {
				LaserType& laser = access<LaserType>(b);
				laser.free();
			};
		}

		/// @brief Despaws all active lasers.
		void despawnAll() override {
			auto const uc = used;
			for (auto b: uc) {
				LaserType& laser = access<LaserType>(b);
				laser.despawn();
			};
		}

		/// @brief Returns the server's laser capacity.
		/// @return Laser capacity.
		usize capacity() override {
			return all.size();
		}

		/// @brief Returns all active lasers in a given area.
		/// @param bound Area to get lasers in.
		/// @return Active lasers in area.
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

		/// @brief Returns all active lasers not in a given area.
		/// @param bound Area to get lasers not in.
		/// @return Active lasers not in area.
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
		/// @brief Returns whether a laser is in the active lasers list.
		/// @param object Laser to check.
		/// @return Whether item exists in active list.
		bool contains(HandleType const& object) override {
			return (used.find(object) != -1);
		}
		
		/// @brief Frees up an laser from use.
		/// @param object Laser to free.
		/// @return Reference to self.
		LaserServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			LaserType& laser = *object.template as<LaserType>();
			if (!laser.isFree()) laser.free();
			AServer::release(object);
			return *this;
		}

	private:
		/// @brief All lasers in the server.
		StaticList<Laser> all;
	};
}

#endif