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
			usize body	= title - 1;
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

		struct LineStep: Co::IAwaitable<void> {
			Instance<Box> box;
			virtual ~LineStep();
			bool await_ready() final	{return false;							}
			bool await_suspend() final	{if (box) box->hide(); return false;	}
			void await_resume() final	{										}
		};

		using ActionStep = Co::Yielder;

		virtual ~Actor() {}
		
		void show() final;
		void hide() final;
		virtual LineStep	say(Line const& what);
		virtual LineStep	add(Line const& what);
		virtual ActionStep	perform(Action const& act);
	};

	struct Scene {
		using Actors	= List<Handle<Actor>>;
		using Cast		= Dictionary<Handle<Actor>>;
		Cast cast;
	};

	using Script = Co::Routine;
}

#endif