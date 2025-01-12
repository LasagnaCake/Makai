#ifndef MAKAILIB_EX_GAME_DANMAKU_CORE_H
#define MAKAILIB_EX_GAME_DANMAKU_CORE_H

#include <makai/makai.hpp>

#include "layers.hpp"

namespace Makai::Ex::Game::Danmaku {
	namespace C2D = Collision::C2D;

	using CollisionServer = C2D::Server;

	template<Type::Ex::Tween::Tweenable T>
	struct Property {
		T					value		= 0;
		bool				interpolate	= false;
		T					start		= 0;
		T					stop		= 0;
		float				speed		= 0;
		Math::Ease::Mode	ease		= Math::Ease::linear;
		float				factor		= 0;

		constexpr T next() {
			if (!interpolate || speed == 0)
				return value;
			factor = Math::clamp<float>(factor, 0, 1);
			if (factor == 0)		value = start;
			else if (factor < 1)	value = Math::lerp<T>(start, stop, ease(factor));
			else					value = stop;
			factor += speed;
			return value;
		}

		constexpr Property& reverse() {
			CTL::swap(start, stop);
			factor = 1 - factor;
		}
	};

	struct PauseState {
		llong	time	= -1;
		bool	enabled	= false;
	};

	struct GameArea {
		Vector2 center;
		Vector2 size;

		constexpr Vector2 topLeft() const		{return center + size * Vector2(-1, +1);	}
		constexpr Vector2 topRight() const		{return center + size * Vector2(+1, +1);	}
		constexpr Vector2 bottomLeft() const	{return center + size * Vector2(-1, -1);	}
		constexpr Vector2 bottomRight() const	{return center + size * Vector2(+1, -1);	}

		constexpr C2D::Box asArea() const		{return C2D::Box(center, size);				}
	};

	struct GameObjectConfig {
		using CollisionMask = GameObject::CollisionMask;
		GameArea&			board;
		GameArea&			playfield;
		CollisionMask const	affects		= {};
		CollisionMask const	affectedBy	= {};
		CollisionMask const	tags		= {};
	};

	struct GameObject {
		using PromiseType			= Makai::Co::Promise<usize, true>;
		using Collider				= CollisionServer::Collider;
		using CollisionArea			= C2D::Area;
		using CollisionDirection	= C2D::Direction;
		using CollisionMask			= CollisionLayer::CollisionMask;

		GameObject(GameObjectConfig const& cfg):
			board(cfg.board),
			playfield(cfg.playfield),
			affects(cfg.affects),
			affectedBy(cfg.affectedBy),
			tags(cfg.tags) {
				bindCollisionHandler(*this);
			}

		virtual ~GameObject() {}

		PromiseType task;

		PauseState pause;

		Math::Transform2D trans;
		
		virtual GameObject& spawn()		= 0;
		virtual GameObject& despawn()	= 0;

		virtual void onUpdate(float) {
			if (!active) return;
			if (pause.enabled && pause.time > 0) {
				--pause.time;
				return;
			} else if (pause.enabled) {
				pause.time		= -1;
				pause.enabled	= false;
				onUnpause();
			}
			if (delay > 0) {
				--delay;
				return;
			}
			while (!delay && task)
				delay = task.next();
		}
		
		bool paused() const {
			if (pause.enabled)
				return pause.time > 0;
			return true;
		}

		virtual void onCollision(Collider const& collider, CollisionDirection const direction) = 0;

	protected:
		GameArea&	board;
		GameArea&	playfield;

		virtual void onUnpause() {}

		void resetCollisionState() {
			collider->affects		= affects;
			collider->affectedBy	= affectedBy;
			collider->tags			= tags;
		}

		static PromiseType doNothing() {co_return 1;}

		bool active = false;

	protected:
		static void bindCollisionHandler(GameObject& self) {
			self.collider->onCollision = [&] (Collider const& collider, CollisionDirection const direction) {
				self.onCollision(collider, direction);
			};
		}

		Handle<CollisionArea> collision() const {
			return collider.asWeak().as<CollisionArea>();
		}

	private:
		Instance<Collider> collider = CollisionServer::createCollider();

		CollisionMask const affects;
		CollisionMask const affectedBy;
		CollisionMask const tags;

		usize delay = 0;
	};

	struct ISpriteContainer {
		virtual ISpriteContainer& setSpriteFrame(Vector2 const& frame)						= 0;
		virtual ISpriteContainer& setSpriteSheetSize(Vector2 const& size)					= 0;
		virtual ISpriteContainer& setSprite(Vector2 const& sheetSize, Vector2 const& frame)	= 0;
		virtual ISpriteContainer& setSpriteRotation(float const angle)						= 0;
	};

	struct AttackObject {
		Property<float>	velocity;
		Property<float>	rotation;
	};

	struct Circular {
		Property<Vector2>	radius;
	};

	struct Glowing {
		bool glowing = false;
	};

	using SpriteInstance	= Instance<Graph::AnimatedPlaneRef>;
	using SpriteHandle		= Handle<Graph::AnimatedPlaneRef>;
}

#endif