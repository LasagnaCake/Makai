#ifndef MAKAILIB_EX_DIALOG_ACTOR_H
#define MAKAILIB_EX_DIALOG_ACTOR_H

#include <makai/makai.hpp>

#include "box.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Actor: IVisible {
		Instance<Box>	dialog;

		virtual ~Actor() {}
		
		void show() final;
		void hide() final;

		virtual void enter()						{show();}
		virtual void leave()						{hide();}
		virtual void stepIn()						{}
		virtual void stepOut()						{}

		virtual void say(Line const& line)			{if (dialog) dialog->display(line);	}
		virtual void add(Line const& line)			{if (dialog) dialog->append(line);	}

		virtual void perform(Action const& action)	{
			switch (Hasher::hash(action.name)) {
				#if __cpp_constexpr == 202306L
				case (Hasher::hash("enter")):		return enter();
				case (Hasher::hash("leave")):		return leave();
				case (Hasher::hash("step-in")):		return stepIn();
				case (Hasher::hash("step-out")):	return stepOut();
				#else
				// enter
				case (0xca6057231f569b15L):	return enter();
				// leave
				case (0x64141541468a5313L): return leave();
				// step-in
				case (0xc60e254656ddb82dL): return stepIn();
				// step-out
				case (0xf8562229bb12ef2aL): return stepOut();
				#endif
			}
		}
	};

	struct ActorRef {
		Handle<Actor> actor;
		void say(Line const& line)			{if (actor) actor->say(line);		}
		void add(Line const& line)			{if (actor) actor->add(line);		}
		void perform(Action const& action)	{if (actor) actor->perform(action);	}
	};
}

#endif