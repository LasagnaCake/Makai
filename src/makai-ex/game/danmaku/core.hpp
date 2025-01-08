#ifndef MAKAILIB_EX_GAME_DANMAKU_CORE_H
#define MAKAILIB_EX_GAME_DANMAKU_CORE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Danmaku {
	using CollisionServer = Makai::Collision::C2D::Server;

	struct Property {
		float				value		= 0;
		bool				interpolate	= false;
		float				start		= 0;
		float				stop		= 0;
		float				speed		= 0;
		Math::Ease::Mode	ease		= Math::Ease::linear;
		float				factor		= 0;

		float next() {
			if (!interpolate || speed == 0)
				return value;
			if (factor == 0)		value = start;
			else if (factor < 1)	value = Math::lerp(start, stop, ease(factor));
			else					value = stop;
			factor += speed;
			return value;
		}
	};

	struct PauseState {
		llong	time	= -1;
		bool	enabled	= false;
	};

	struct GameObject: CollisionServer::ICollider, Makai::IUpdateable {
		using PromiseType			= Makai::Co::Promise<usize, true>;
		using Collider				= CollisionServer::ICollider;
		using CollisionDirection	= Collision::C2D::Direction;

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

		Functor<void(GameObject&, float, App&)> onObjectUpdate;

	protected:
		static PromiseType doNothing() {co_return 1;}

		bool active = false;

	private:
		usize delay = 0;
	};

	struct AttackObject: GameObject {
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

		Property velocity;
		Property rotation;

		bool discardable	= true;
		bool dope			= true;

		virtual AttackObject& reset()							= 0;
		virtual AttackObject& clear()							= 0;
		virtual AttackObject& discard(bool const force = false)	= 0;

		Functor<void(AttackObject&, Action const)>	onAction;

		virtual bool isFree() const = 0;

		AttackObject& free()	{setFree(true);		}
		AttackObject& enable()	{setFree(false);	}

	protected:
		virtual AttackObject& setFree(bool const state) = 0;
	};
}

#endif