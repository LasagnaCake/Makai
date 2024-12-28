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
}

#endif