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
			radius			= {};
			scale			= {};
			sprite			= {};
			glowing			= false;
			rotateSprite	= true;
			dope			= false;
			animColor		= Graph::Color::WHITE;
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
}

#endif