#ifndef MAKAILIB_EX_DIALOG_ACTOR_H
#define MAKAILIB_EX_DIALOG_ACTOR_H

#include <makai/makai.hpp>

#include "box.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Actor: IVisible {
		Graphic			body;
		Instance<Box>	dialog;

		struct LineStep: Co::Consumer {
			Instance<Box> box;
			LineStep(Instance<Box>&& box):		box(CTL::move(box))	{}
			LineStep(Instance<Box> const& box):	box(box)			{}
		private:
			void onExit() override final {box->hide();}
		};

		using ActionStep = Co::Consumer;

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