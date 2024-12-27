#ifndef MAKAILIB_EX_DIALOG_ACTOR_H
#define MAKAILIB_EX_DIALOG_ACTOR_H

#include <makai/makai.hpp>

#include "box.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Actor: IVisible {
		Graphic			body;
		Instance<Box>	dialog;

		struct LineStep {
			Instance<Box> box;
			LineStep(Instance<Box>&& box):		box(CTL::move(box))	{}
			LineStep(Instance<Box> const& box):	box(box)			{}
			bool await_ready()		{return consume();	}
			void await_suspend()	{					}
			void await_resume()		{					}
		private:
			bool consumed = false;
			bool consume() {
				if (!consumed)
					return consumed = true;
				box->hide();
				return false;
			}
		};

		struct ActionStep {
			bool await_ready() 		{return consume();	}
			void await_suspend()	{					}
			void await_resume()		{					}
		private:
			bool consumed = false;
			bool consume() {
				if (!consumed)
					return consumed = true;
				return false;
			}
		};

		virtual ~Actor() {}
		
		void show() final;
		void hide() final;
		virtual LineStep	say(Line const& line);
		virtual LineStep	add(Line const& line);
		virtual ActionStep	perform(Action const& act);
	};

	struct ActorRef {
		using LineStep		= Actor::LineStep;
		using ActionStep	= Actor::ActionStep;
		Handle<Actor> actor;
		LineStep	say(Line const& what)		{if (actor) return actor->say(what);	return {nullptr};	}
		LineStep	add(Line const& what)		{if (actor) return actor->add(what);	return {nullptr};	}
		ActionStep	perform(Action const& act)	{if (actor) return actor->perform(act);	return {};			}
	};
}

#endif