#ifndef MAKAILIB_EX_GAME_DIALOG_BOX_H
#define MAKAILIB_EX_GAME_DIALOG_BOX_H

#include <makai/makai.hpp>

#include "core.hpp"

namespace Makai::Ex::Game::Dialog {
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
		virtual void append(Line const& content);
	};
}

#endif