#ifndef MAKAILIB_EX_GAME_DIALOG_CORE_H
#define MAKAILIB_EX_GAME_DIALOG_CORE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game::Dialog {
	using Graphic = Instance<Graph::IGraphic>;

	struct Content {
		String	content;
		Vector4	color	= Graph::Color::WHITE;
	};

	struct Line {
		Content	title;
		Content	body;
	};

	struct Method {
		usize		hash;
		StringList	params;
	};
	
	struct Action:	Method	{};
	struct Emotion:	Method	{};

	struct IPerformer {
		virtual ~IPerformer() {}

		virtual void color(Vector4 const& color)	= 0;
		virtual usize say(Content const& line)		= 0;
		virtual usize add(Content const& line)		= 0;
		virtual usize perform(Action const& action)	= 0;
		virtual usize emote(Emotion const& emotion)	= 0;
	};
}

#endif