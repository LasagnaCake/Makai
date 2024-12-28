#ifndef MAKAILIB_EX_DIALOG_ACTOR_H
#define MAKAILIB_EX_DIALOG_ACTOR_H

#include <makai/makai.hpp>

#include "box.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Actor: IVisible {
		Instance<Box>	dialog;

		virtual ~Actor() {}
		
		void show() override {dialog->show();}
		void hide() override {dialog->hide();}

		virtual void enter()	{show();			}
		virtual void leave()	{hide();			}
		virtual void stepIn()	{dialog->show();	}
		virtual void stepOut()	{dialog->hide();	}

		virtual void say(Content const& line)	{if (dialog) dialog->setBody(line);		}
		virtual void add(Content const& line)	{if (dialog) dialog->appendBody(line);	}

		virtual void say(Line const& line)		{if (dialog) dialog->display(line);		}
		virtual void add(Line const& line)		{if (dialog) dialog->append(line);		}
		
		virtual usize perform(Action const& action) {
			switch (action.hash) {
				#if __cpp_constexpr == 202306L
				case (Hasher::hash("enter")):		enter();	return 0;
				case (Hasher::hash("leave")):		leave();	return 0;
				case (Hasher::hash("step-in")):		stepIn();	return 0;
				case (Hasher::hash("step-out")):	stepOut();	return 0;
				#else
				// enter
				case (0xca6057231f569b15L): enter();	return 0; 
				// leave
				case (0x64141541468a5313L): leave();	return 0;
				// step-in
				case (0xc60e254656ddb82dL): stepIn();	return 0;
				// step-out
				case (0xf8562229bb12ef2aL): stepOut();	return 0;
				#endif
			}
			return 0;
		}

		virtual usize emote(Emotion const& emotion)	{
			return 0;
		}
	};

	struct ActorRef {
		Handle<Actor> actor;
		void say(Content const& line)	{if (actor) actor->say(line);	}
		void add(Content const& line)	{if (actor) actor->add(line);	}
		void say(Line const& line)		{if (actor) actor->say(line);	}
		void add(Line const& line)		{if (actor) actor->add(line);	}
		usize perform(Action const& action) {
			if (actor)
				return actor->perform(action);
			return 0;
		}
		usize emote(Emotion const& emotion) {
			if (actor)
				return actor->emote(emotion);
			return 0;
		}
	};
}

#endif