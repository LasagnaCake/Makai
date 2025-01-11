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

	struct GameObject {
		using PromiseType			= Makai::Co::Promise<usize, true>;
		using Collider				= CollisionServer::Collider;
		using CollisionArea			= C2D::Area;
		using CollisionDirection	= C2D::Direction;
		using CollisionMask			= CollisionLayer::CollisionMask;

		GameObject(CollisionMask const& affects, CollisionMask const& affectedBy, CollisionMask const& tags):
			affects(affects),
			affectedBy(affectedBy),
			tags(tags) {
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

	struct Playfield {
		Vector2 center;
		Vector2 size;

		constexpr Vector2 topLeft()		{return center + size * Vector2(-1, +1);}
		constexpr Vector2 topRight()	{return center + size * Vector2(+1, +1);}
		constexpr Vector2 bottomLeft()	{return center + size * Vector2(-1, -1);}
		constexpr Vector2 bottomRight()	{return center + size * Vector2(+1, -1);}
	};

	using SpriteInstance	= Instance<Graph::AnimatedPlaneRef>;
	using SpriteHandle		= Handle<Graph::AnimatedPlaneRef>;
}

#endif