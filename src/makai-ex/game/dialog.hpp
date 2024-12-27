#ifndef MAKAILIB_EX_GAME_DIALOG_H
#define MAKAILIB_EX_GAME_DIALOG_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog {
	using Graphic = Handle<Graph::IGraphic>;

	struct Content {
		String	content;
		Vector4	color	= Graph::Color::WHITE;
	};

	struct Message {
		Content	title;
		Content	text;
		usize	duration	= 600;
		bool	autoplay	= false;
	};

	struct Action {
		String name;
	};

	struct SpeechBubble: IVisible {
		Graph::Label	title;
		Graph::Label	text;
		Graphic			body;

		virtual ~SpeechBubble() {}
		void show() final;
		void hide() final;
		virtual void display(Content const& title, Content const& text);
	};

	struct Actor: IVisible {
		using Step = Co::AlwaysSuspend;
		Graphic					body;
		Handle<SpeechBubble>	bubble;

		virtual ~Actor() {}
		
		virtual Step enter();
		virtual Step leave();
		void show() final;
		void hide() final;
		Step say(Message const& what);
		Step act(Action const& what);
	};

	struct Scene {
	};

	using Script = Co::Routine;
}

#endif