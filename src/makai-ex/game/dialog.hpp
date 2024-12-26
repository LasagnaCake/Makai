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

	struct SpeechBubble {
		Graph::Label	title;
		Graph::Label	text;
		Graphic			body;

		void display(Content const& title, Content const& text);
	};

	struct Actor {
		Graphic			body;
		SpeechBubble	bubble;
		CTL::Co::AlwaysSuspend enter();
		CTL::Co::AlwaysSuspend leave();
		CTL::Co::AlwaysSuspend say(Message const& what);
	};

	struct Scene {
	};

	using Script = CTL::Co::Routine;
}

#endif