#ifndef MAKAILIB_EX_GAME_DIALOG_H
#define MAKAILIB_EX_GAME_DIALOG_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog {
	using Graphic = Instance<Graph::IGraphic>;

	struct Content {
		String	content;
		Vector4	color	= Graph::Color::WHITE;
	};

	struct Line {
		Content	title;
		Content	text;
	};

	struct Action {
		String name;
	};

	struct Box: IVisible {
		Graph::Label	title;
		Graph::Label	text;
		Graphic			body;

		struct Layers {
			usize title;
			usize text	= title;
		//	usize body	= title - 1;
		};

		virtual ~Box() {}

		void setRenderLayers(Layers const& layers);

		void show() final;
		void hide() final;
		virtual void display(Line const& content);
	};

	struct Actor: IVisible {
		Graphic			body;
		Instance<Box>	dialog;

		struct LineStep {
			Instance<Box> box;
			LineStep(Instance<Box>&& box):		box(CTL::move(box))	{}
			LineStep(Instance<Box> const& box):	box(box)			{}
			virtual ~LineStep();
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
			virtual ~ActionStep();
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
		virtual LineStep	say(Line const& what);
		virtual LineStep	add(Line const& what);
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

	struct Scene {
		using Actors	= List<Handle<Actor>>;
		using Cast		= Dictionary<Handle<Actor>>;
		Cast cast;

		using Step = Co::Yielder;

		Step begin();
		Step end();

		ActorRef actor(String const& name) {
			if (cast.contains(name))
				return {cast.at(name)};
			return {nullptr};
		}

		void highlight(StringList const& actors);
		void detract(StringList const& actors);
		void redirect(StringList const& actors);

		void highlight(Actors const& actors);
		void detract(Actors const& actors);
		void redirect(Actors const& actors);

		void highlight(String const& actor);
		void detract(String const& actor);
		void redirect(String const& actor);

		void order(StringList const& actors, Action const& action);
		void order(Actors const& actors, Action const& action);

		virtual void highlight(Actor const& actor);
		virtual void detract(Actor const& actor);
		virtual void redirect(Actor const& actor);
	};

	struct Player: IUpdateable, Co::IRoutineTask {
		using Script = Co::IRoutineTask::PromiseType;

		Player() {stop();}

		Script task() override {return script();}

		void onUpdate(float, App&) {
			if (isFinished || paused) return;
			IRoutineTask::process();
			if (!dialog) isFinished = true;
		}

		virtual Script script() = 0;

	private:
		Script dialog;
	};
}

#endif