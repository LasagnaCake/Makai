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

		virtual ~Box() {}

		void show() final;
		void hide() final;
		virtual void display(Line const& content);
	};

	struct Actor: IVisible {
		using Step = Co::Yielder;
		Graphic			body;
		Instance<Box>	dialog;

		virtual ~Actor() {}
		
		void show() final;
		void hide() final;
		virtual Step say(Line const& what);
		virtual Step act(Action const& action);
	};

	struct Scene {
		using Actors	= List<Handle<Actor>>;
		using Cast		= Dictionary<Instance<Actor>>;
		Cast cast;
	};

	using Script = Co::Routine;
}

#endif