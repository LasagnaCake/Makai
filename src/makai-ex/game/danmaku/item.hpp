#ifndef MAKAILIB_EX_GAME_DANMAKU_ITEM_H
#define MAKAILIB_EX_GAME_DANMAKU_ITEM_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct Item;
	struct ItemConfig;

	template<class T = Item, class C = ItemConfig> struct ItemServer;

	struct ItemConfig: ServerObjectConfig, GameObjectConfig {
		using GameObjectConfig::CollisionMask;
		struct Collision {
			CollisionMask const player = Danmaku::Collision::Tag::FOR_PLAYER_1;
		} const mask;
	};

	struct Item: AServerObject, ISpriteContainer, Weighted, Circular, Glowing, Dope, RotatesSprite, Magnetizable {
		Item(ItemConfig const& cfg):
			AServerObject(cfg), mask(cfg.mask), server(cfg.server) {
			collision()->shape = shape.template as<C2D::IBound2D>();
		}

		virtual ~Item() {}

		Item& clear() override {
			AServerObject::clear();
			rotateSprite		= true;
			dope				= true;
			jumpy				= false;
			glowOnSpawn			= false;
			radius				= {1};
			sprite				= {};
			gravity				= {1};
			terminalVelocity	= {1};
			magnet				= {};
			glow				= {};
			id					= 0;
			value				= 1;
			animColor			= Graph::Color::WHITE;
			counter				= 0;
			spawnglow			= 0;
			acceleration		= 0;
			internalRotation	= 0;
			initSprites();
			return *this;
		}

		Item& reset() override {
			if (isFree()) return *this;
			AServerObject::reset();
			radius.factor			= 0;
			scale.factor			= 0;
			gravity.factor			= 0;
			terminalVelocity.factor	= 0;
			magnet.strength.factor	= 0;
			return *this;
		}

		void onUpdate(float delta) override {
			if (isFree()) return;
			AServerObject::onUpdate(delta);
			updateSprite(mainSprite.asWeak());
			updateSprite(glowSprite.asWeak(), true);
			updateHitbox();
			animate();
			if (paused()) return;
			color.next();
			radius.next();
			glow.next();
			terminalVelocity.next();
			acceleration += gravity.next();
			if (!jumpy) {
				auto const tva = terminalVelocity.value.absolute();
				acceleration.clamp(-tva, tva);
			}
			if (magnet.enabled && magnet.target && objectState == State::SOS_ACTIVE)
				trans.position += trans.position.normalTo(*magnet.target) * magnet.strength.next() * delta;
			else if (jumpy) {
				auto const tva = terminalVelocity.value.absolute();
				if (acceleration.x > +tva.x) acceleration.x = (acceleration.x * -1) - gravity.value.x;
				if (acceleration.y > +tva.y) acceleration.y = (acceleration.y * -1) - gravity.value.y;
				if (acceleration.x < -tva.x) acceleration.x = (acceleration.x * -1) + gravity.value.x;
				if (acceleration.y < -tva.y) acceleration.y = (acceleration.y * -1) + gravity.value.y;
				trans.position += acceleration * delta;
			} else trans.position += acceleration * delta;
			trans.scale		= scale.next();
			playfieldCheck();
		}

		Item& discard(bool const immediately = false, bool const force = false) override {
			if (isFree()) return *this;
			if (!discardable && !force) return *this;
			if (!immediately)	despawn();
			else				free();
			return *this;
		}

		Item& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			animColor.a = 0;
			counter = 0;
			internalRotation = 0;
			objectState = State::SOS_SPAWNING;
			onAction(*this, Action::SOA_SPAWN_BEGIN);
			return *this;
		}

		Item& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			animColor.a = 1;
			counter = 0;
			internalRotation = 0;
			objectState = State::SOS_DESPAWNING;
			onAction(*this, Action::SOA_DESPAWN_BEGIN);
			return *this;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
		}

		Item& setSpriteRotation(float const angle) override {
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

		Item& setFree(bool const state) override {
			if (state) {
				active = false;
				hideSprites();
				objectState = State::SOS_FREE;
				clear();
				release(this, server);
			} else {
				setCollisionState(false);
				active = true;
				showSprites();
				objectState = State::SOS_ACTIVE;
			}
			return *this;
		}
		
		ItemConfig::Collision const mask;

		usize id	= 0;
		usize value	= 1;

		bool jumpy = false;

	private:
		AServer&	server;

		float		internalRotation = 0;

		SpriteInstance mainSprite	= nullptr;
		SpriteInstance glowSprite	= nullptr;

		Vector2 acceleration = 0;

		usize counter	= 0;
		float spawnglow	= 0;

		Vector4 animColor = Graph::Color::WHITE;

		Instance<C2D::Circle> shape = new C2D::Circle(0);

		template <class, class> friend class ItemServer;

		void setSpriteVisibility(bool const setGlowSprite, bool const state) {
			if (glowSprite && setGlowSprite)	glowSprite->visible	= state; 
			else if (mainSprite)				mainSprite->visible	= state;
		}

		void hideSprites() {
			if (glowSprite)	glowSprite->visible	= false; 
			if (mainSprite)	mainSprite->visible	= false;
		}

		void showSprites() {
			if (glowSprite)	glowSprite->visible	= true; 
			if (mainSprite)	mainSprite->visible	= true;
		}
		
		void initSprites() {
			if (mainSprite) mainSprite->local.scale = 0;
			if (glowSprite) glowSprite->local.scale = 0;
		}

		void updateSprite(SpriteHandle const& sprite, bool glowSprite = false) {
			if (!sprite) return;
			sprite->visible = true;
			sprite->frame	= this->sprite.frame;
			sprite->size	= this->sprite.sheetSize;
			if (rotateSprite)
				sprite->local.rotation.z	= trans.rotation + internalRotation;
			sprite->local.position			= Vec3(trans.position, sprite->local.position.z);
			sprite->local.scale				= trans.scale;
			float const iglow = glowOnSpawn ? Math::lerp<float>(1, glow.value, spawnglow) : glow.value;
			auto const glowFX = Graph::Color::alpha(glowSprite ? iglow : 1 - iglow);
			sprite->setColor(animColor * color.value * glowFX);
		}

		void updateHitbox() {
			if (shape) {
				shape->radius	= radius.value * trans.scale;
				shape->position	= trans.position;
				shape->rotation	= trans.rotation + internalRotation;
			}
		}

		void animate() {
			switch (objectState) {
				case State::SOS_DESPAWNING: {
					if (counter++ < despawnTime) {
						spawnglow = counter / static_cast<float>(despawnTime);
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
						internalRotation = (1.0 - counter / static_cast<float>(spawnTime));
						internalRotation *= TAU;
					} else {
						internalRotation = 0;
						counter = 0;
						onAction(*this, Action::SOA_DESPAWN_END);
						free();
					}
				} break;
				case State::SOS_SPAWNING: {
					if (counter++ < spawnTime) {
						spawnglow = 1.0 - counter / static_cast<float>(despawnTime);
						animColor.a = counter / static_cast<float>(spawnTime);
						internalRotation = (counter / static_cast<float>(spawnTime));
						internalRotation *= TAU;
					} else {
						internalRotation = 0;
						counter = 0;
						setCollisionState(true);
						onAction(*this, Action::SOA_SPAWN_END);
						objectState = State::SOS_ACTIVE;
					}
				} break;
				[[likely]]
				default: break;
			}
		}

		void playfieldCheck() {
			if (!dope) return;
			auto const
				min = playfield.min(),
				max = playfield.max()
			;
			if (
				trans.position.x < min.x
			||	trans.position.x > max.x
			||	trans.position.y < min.y
			) free();
		}
	};

	struct ItemCollisionConfig {
		ColliderConfig const colli = {
			Danmaku::Collision::Layer::ITEM,
			Collision::Tag::FOR_PLAYER_1
		};
		CollisionLayerConfig const layer = {
			Danmaku::Collision::Mask::ITEM,
			{}
		};
		ItemConfig::Collision const mask = {};
	};

	struct ItemServerConfig:
		ServerConfig,
		ServerMeshConfig,
		ServerGlowMeshConfig,
		BoundedObjectConfig,
		ItemCollisionConfig {};

	struct ItemServerInstanceConfig: ServerConfig, ItemCollisionConfig {};

	template<Type::Derived<Item> TItem, Type::Derived<ItemConfig> TConfig>
	struct ItemServer<TItem, TConfig>: AServer, AUpdateable {
		using CollisionMask = AGameObject::CollisionMask;

		using ItemType		= TItem;
		using ConfigType	= TConfig;

		Graph::ReferenceHolder& mainMesh;
		Graph::ReferenceHolder& glowMesh;

		GameArea& board;
		GameArea& playfield;

		ItemServer(ItemServerConfig const& cfg):
			mainMesh(cfg.mainMesh),
			glowMesh(cfg.glowMesh),
			board(cfg.board),
			playfield(cfg.playfield) {
			auto& cl		= CollisionServer::layers[cfg.colli.layer];
			cl.affects		= cfg.layer.affects;
			cl.affectedBy	= cfg.layer.affectedBy;
			all.resize(cfg.size);
			free.resize(cfg.size);
			used.resize(cfg.size);
			for (usize i = 0; i < cfg.size; ++i) {
				float const zoff = i / static_cast<float>(cfg.size);
				all.constructBack(ConfigType{*this, cfg, cfg.colli, cfg.mask});
				all.back().mainSprite = mainMesh.createReference<Graph::AnimatedPlaneRef>();
				all.back().mainSprite->local.position.z = -zoff;
				if (&cfg.mainMesh != &cfg.glowMesh) {
					all.back().glowSprite = glowMesh.createReference<Graph::AnimatedPlaneRef>();
					all.back().glowSprite->local.position.z = -zoff;
				}
				all.back().hideSprites();
				all.back().setCollisionState(false);
				free.pushBack(&all.back());
			}
		}

		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Reference<ItemType> item = b.template as<ItemType>();
				item->clear();
				item->enable();
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
				ItemType& item = access<ItemType>(b);
				item.discard();
			};
		}
		
		void freeAll() override {
			for (auto b: used) {
				ItemType& item = access<ItemType>(b);
				item.free();
			};
		}

		void despawnAll() override {
			for (auto b: used) {
				ItemType& item = access<ItemType>(b);
				item.despawn();
			};
		}

		usize capacity() override {
			return all.size();
		}

		ObjectQueryType getInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				ItemType& item = access<ItemType>(b);
				if (
					item.shape
				&&	C2D::withinBounds(*item.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

		ObjectQueryType getNotInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				ItemType& item = access<ItemType>(b);
				if (
					item.shape
				&&	!C2D::withinBounds(*item.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

	protected:
		bool contains(HandleType const& object) override {
			return (used.find(object) != -1);
		}
		
		ItemServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			ItemType& item = *(object.template as<ItemType>());
			if (!item.isFree()) item.free();
			AServer::release(object);
			return *this;
		}

	private:
		StaticList<ItemType> all;
	};
}

#endif