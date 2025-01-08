#ifndef MAKAILIB_EX_GAME_DANMAKU_CORE_H
#define MAKAILIB_EX_GAME_DANMAKU_CORE_H

#include <makai/makai.hpp>

#include "layers.hpp"

namespace Makai::Ex::Game::Danmaku {
	using CollisionServer = Makai::Collision::C2D::Server;

	template<class T>
	struct Property {
		T					value		= 0;
		bool				interpolate	= false;
		T					start		= 0;
		T					stop		= 0;
		T					speed		= 0;
		Math::Ease::Mode	ease		= Math::Ease::linear;
		T					factor		= 0;

		constexpr T next() {
			if (!interpolate || speed == 0)
				return value;
			if (factor == 0)		value = start;
			else if (factor < 1)	value = Math::lerp<T>(start, stop, ease(factor));
			else					value = stop;
			factor += speed;
			return value;
		}
	};

	struct PauseState {
		llong	time	= -1;
		bool	enabled	= false;
	};

	struct GameObject: Makai::IUpdateable {
		using PromiseType			= Makai::Co::Promise<usize, true>;
		using Collider				= CollisionServer::Collider;
		using CollisionArea			= Collision::C2D::Area;
		using CollisionDirection	= Collision::C2D::Direction;
		using CollisionMask			= CollisionLayer::CollisionMask;

		GameObject(CollisionMask const& affects, CollisionMask const& affectedBy):
			affects(affects),
			affectedBy(affectedBy) {
				bindCollisionHandler(*this);
			}

		virtual ~GameObject() {}

		PromiseType task;

		PauseState pause;

		Math::Transform2D trans;
		
		virtual GameObject& spawn()		= 0;
		virtual GameObject& despawn()	= 0;

		void onUpdate(float, Makai::App&) override {
			if (!active) return;
			if (pause.enabled && pause.time > 0) {
				--pause.time;
				return;
			}
			pause.enabled = false;
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
		static void bindCollisionHandler(GameObject& self) {
			self.collider->onCollision = [&] (Collider const& collider, CollisionDirection const direction) {
				self.onCollision(collider, direction);
			};
		}

		Instance<Collider> collider = CollisionServer::createCollider();

		void resetCollisionLayers() {
			collider->affects		= affects;
			collider->affectedBy	= affectedBy;
		}

		static PromiseType doNothing() {co_return 1;}

		bool active = false;

	private:
		const CollisionMask affects;
		const CollisionMask affectedBy;

		usize delay = 0;
	};

	struct AttackObject: GameObject {
		using GameObject::GameObject;

		virtual ~AttackObject() {}

		enum class State {
			AOS_FREE,
			AOS_SPAWNING,
			AOS_ACTIVE,
			AOS_DESPAWNING
		};
		
		enum class Action {
			AOA_SPAWN_BEGIN,
			AOA_SPAWN_END,
			AOA_DESPAWN_BEGIN,
			AOA_DESPAWN_END,
			AOA_UNPAUSE,
			AOA_DISCARD
		};

		Property<float> velocity;
		Property<float> rotation;

		bool discardable	= true;
		bool dope			= true;

		virtual AttackObject& clear() {
			trans					= Transform2D();
			velocity				= {};
			rotation				= {};
			dope					= true;
			discardable				= true;
			collider->shape			= nullptr;
			collider->canCollide	= true;
			task					= doNothing();
			pause					= {};
			onAction.clear();
			onObjectUpdate.clear();
			resetCollisionLayers();
		}

		virtual AttackObject& reset() {
			velocity.value	= velocity.start;
			rotation.value	= rotation.start;
			velocity.factor	= 0;
			rotation.factor	= 0;
		}

		virtual AttackObject& discard(bool const force = false)	= 0;

		Functor<void(AttackObject&, Action const)>	onAction;
		Functor<void(AttackObject&, float, App&)>	onObjectUpdate;

		virtual bool isFree() const = 0;

		AttackObject& free()	{setFree(true);		}
		AttackObject& enable()	{setFree(false);	}

		Instance<CollisionArea> collision() const {
			return collider.as<CollisionArea>();
		}

	protected:
		virtual AttackObject& setFree(bool const state) = 0;
	};

	using SpriteInstance	= Instance<Graph::AnimatedPlaneRef>;
	using SpriteHandle		= Handle<Graph::AnimatedPlaneRef>;
}

#endif