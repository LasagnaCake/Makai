#ifndef MAKAILIB_EX_GAME_DANMAKU_ITEM_H
#define MAKAILIB_EX_GAME_DANMAKU_ITEM_H

#include "core.hpp"
#include "server.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct ItemServer;

	struct ItemConfig: ServerObjectConfig, GameObjectConfig {

	};

	struct Item: AServerObject, ISpriteContainer, Weighted, Circular, Glowing, Dope, RotatesSprite {
		Item(ItemConfig const& cfg):
			AServerObject(cfg), server(cfg.server) {
			collision()->shape = shape.as<C2D::IBound2D>();
		}

		Item& clear() override {
			if (isFree()) return *this;
			AServerObject::clear();
			radius				= {};
			scale				= {};
			sprite				= {};
			gravity				= {};
			terminalVelocity	= {};
			glowing				= false;
			rotateSprite		= true;
			dope				= false;
			animColor			= Graph::Color::WHITE;
			return *this;
		}

		Item& reset() override {
			if (isFree()) return *this;
			AServerObject::reset();
			gravity.factor			= 0;
			terminalVelocity.factor	= 0;
			radius.factor			= 0;
			scale.factor			= 0;
			return *this;
		}

		void onUpdate(float delta) override {
			if (isFree()) return;
			AServerObject::onUpdate(delta);
			hideSprites();
			setSpriteVisibility(glowing, true);
			updateSprite(mainSprite.asWeak());
			updateSprite(glowSprite.asWeak());
			updateHitbox();
			animate();
			if (paused()) return;
			color.next();
			radius.next();
			terminalVelocity.next();
			acceleration += gravity.next();
			processMax(acceleration.x, terminalVelocity.value.x);
			processMax(acceleration.y, terminalVelocity.value.y);
			trans.position	+= acceleration;
			trans.position	+= gravity.next() * terminalVelocity.next() * delta;
			trans.scale		= scale.next();
			playfieldCheck();
		}

		Item& discard(bool const force = false) override {
			if (isFree()) return *this;
			if (discardable && !force) return *this;
			despawn();
			return *this;
		}

		Item& spawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = State::SOS_SPAWNING;
			onAction(*this, Action::SOA_SPAWN_BEGIN);
			return *this;
		}

		Item& despawn() override {
			if (isFree()) return *this;
			setCollisionState(false);
			counter = 0;
			objectState = State::SOS_DESPAWNING;
			onAction(*this, Action::SOA_DESPAWN_BEGIN);
			return *this;
		}

		void onCollision(Collider const& collider, CollisionDirection const direction) override {
			if (isFree()) return;
			if (collider.tags.match(CollisionTag::BULLET_ERASER).overlap())
				discard();
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
				release(this, server);
			} else {
				active = true;
				objectState = State::SOS_ACTIVE;
			}
			return *this;
		}

		bool rotateSprite = true;

	private:
		AServer&	server;

		float		internalRotation = 0;

		SpriteInstance mainSprite	= nullptr;
		SpriteInstance glowSprite	= nullptr;

		Vector2 acceleration = 0;

		usize counter	= 0;
		bool spawnglow	= false;

		Vector4 animColor = Graph::Color::WHITE;

		Instance<C2D::Circle> shape = new C2D::Circle(0);

		friend class ItemServer;

		constexpr static void processMax(float& value, float const max) {
			if (value > abs(max) || value < -abs(max))
				value = max;
		}

		void setSpriteVisibility(bool const setGlowSprite, bool const state) {
			if (glowSprite && setGlowSprite)	glowSprite->visible	= state; 
			else if (mainSprite)				mainSprite->visible	= state;
		}

		void hideSprites() {
			if (glowSprite)	glowSprite->visible	= false; 
			if (mainSprite)	mainSprite->visible	= false;
		}

		void updateSprite(SpriteHandle const& sprite) {
			if (!sprite) return;
			sprite->frame	= this->sprite.frame;
			sprite->size	= this->sprite.sheetSize;
			if (rotateSprite)
				sprite->local.rotation.z	= trans.rotation + internalRotation;
			sprite->local.position			= Vec3(trans.position, sprite->local.position.z);
			sprite->local.scale				= trans.scale;
			sprite->setColor(animColor * color.value);
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
						spawnglow = true;
						animColor.a = 1.0 - counter / static_cast<float>(despawnTime);
						internalRotation = (1.0 - counter / static_cast<float>(spawnTime));
						internalRotation *= TAU * 3;
					} else {
						spawnglow = false;
						internalRotation = 0;
						onAction(*this, Action::SOA_DESPAWN_END);
						free();
					}
				} break;
				case State::SOS_SPAWNING: {
					if (counter++ < spawnTime) {
						spawnglow = true;
						animColor.a = counter / static_cast<float>(spawnTime);
						internalRotation = (counter / static_cast<float>(spawnTime));
						internalRotation *= TAU * 3;
					} else {
						spawnglow = false;
						internalRotation = 0;
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
				tl = playfield.topLeft(),
				br = playfield.bottomRight()
			;
			if (
				trans.position.x < tl.x
			||	trans.position.x > br.x
			||	trans.position.y > br.y
			) free();
		}
	};

	struct ItemServerConfig: ServerConfig, ServerMeshConfig, ServerGlowMeshConfig, GameObjectConfig {};

	struct ItemServer: AServer, AUpdateable {
		using CollisionMask = AGameObject::CollisionMask;

		Graph::ReferenceHolder& mainMesh;
		Graph::ReferenceHolder& glowMesh;

		GameArea& board;
		GameArea& playfield;

		ItemServer(ItemServerConfig const& cfg):
			mainMesh(cfg.mainMesh),
			glowMesh(cfg.glowMesh),
			board(cfg.board),
			playfield(cfg.playfield) {
			all.resize(cfg.size);
			free.resize(cfg.size);
			used.resize(cfg.size);
			for (usize i = 0; i < cfg.size; ++i) {
				float const zoff = i / static_cast<float>(cfg.size);
				all.pushBack(Item({*this, cfg}));
				all.back().mainSprite = mainMesh.createReference<Graph::AnimatedPlaneRef>();
				all.back().mainSprite->local.position.z = zoff;
				if (&cfg.mainMesh != &cfg.glowMesh) {
					all.back().glowSprite = glowMesh.createReference<Graph::AnimatedPlaneRef>();
					all.back().glowSprite->local.position.z = zoff;
				}
				free.pushBack(&all.back());
			}
		}

		HandleType acquire() override {
			if (auto b = AServer::acquire()) {
				Handle<Item> item = b.polymorph<Item>();
				item->setFree(false);
				item->clear();
				return item.as<AGameObject>();
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
				Item& item = access<Item>(b);
				item.discard();
			};
		}
		
		void freeAll() override {
			for (auto b: used) {
				Item& item = access<Item>(b);
				item.free();
			};
		}

		void despawnAll() override {
			for (auto b: used) {
				Item& item = access<Item>(b);
				item.despawn();
			};
		}

		usize capacity() override {
			return all.size();
		}

		ObjectQueryType getInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				Item& item = access<Item>(b);
				if (
					item.shape
				&&	Collision::GJK::check(*item.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

		ObjectQueryType getNotInArea(C2D::IBound2D const& bound) override {
			ObjectQueryType query;
			for (auto b: used) {
				Item& item = access<Item>(b);
				if (
					item.shape
				&&	!Collision::GJK::check(*item.shape, bound)
				) query.pushBack(b);
			}
			return query;
		}

	protected:
		ItemServer& release(HandleType const& object) override {
			if (used.find(object) == -1) return *this;
			Item& item = *(object.as<Item>());
			AServer::release(object);
			return *this;
		}

	private:
		List<Item> all;
	};
}

#endif