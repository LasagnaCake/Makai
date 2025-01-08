#ifndef MAKAILIB_EX_GAME_DANMAKU_CORE_H
#define MAKAILIB_EX_GAME_DANMAKU_CORE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Danmaku {
	using CollisionServer = Makai::Collision::C2D::Server;

	struct Property {
		bool				enabled;
		float				from;
		float				to;
		float				by;
		Math::Ease::Mode	ease	= Math::Ease::linear;
		float				current	= 0;
	};

	struct PauseState {
		llong	time	= -1;
		bool	enabled	= false;
	};

	struct GameObject: CollisionServer::ICollider, Makai::IUpdateable {
		using PromiseType = Makai::Co::Promise<usize, true>;

		virtual ~GameObject() {}

		PromiseType task;

		PauseState pause;

		Math::Transform2D trans;
		
		virtual GameObject& spawn()		= 0;
		virtual GameObject& despawn()	= 0;

		void onUpdate(float, Makai::App&) override {
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

		Functor<void(GameObject&, float, App&)> onObjectUpdate;

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

		virtual bool isFree() = 0;

		AttackObject& free()	{setFree(true);		}
		AttackObject& enable()	{setFree(false);	}

	protected:
		virtual AttackObject& setFree(bool const state) = 0;
	};
}

#endif