#ifndef MAKAILIB_EX_GAME_DIALOG_H
#define MAKAILIB_EX_GAME_DIALOG_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog {
	struct Actor {
		struct Position {
			Math::Vector2	talking;
			Math::Vector2	rest;
			Math::Vector2	out;
		} position;
		Handle<Graph::IGLDrawable> body;
	};

	struct ActorRef {
		String	name;
		String	face;
		bool	leaving = false;
	};

	using Actors = List<ActorRef>;

	struct Message {
		struct Content {
			String	content;
			Vector4	color	= Graph::Color::WHITE;
		};
		Actors				actors;
		Content				title;
		Content				text;
		Math::Ease::Mode	mode		= Math::Ease::linear;
		usize				duration	= 600;
		bool				autoplay	= false;
	};

	using Messages	= List<Message>;
	using Cast		= Dictionary<Actor>;
}

#endif