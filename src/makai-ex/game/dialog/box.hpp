#ifndef MAKAILIB_EX_GAME_DIALOG_BOX_H
#define MAKAILIB_EX_GAME_DIALOG_BOX_H

#include <makai/makai.hpp>

#include "core.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Box: IVisible {
		Graph::Label	title;
		Graph::Label	text;

		struct Layers {
			usize title;
			usize text	= title;
		};

		virtual ~Box() {}

		virtual void setRenderLayers(Layers const& layers);

		void show() override;
		void hide() override;
		
		virtual void display(Line const& content);
		virtual void append(Line const& content);
	};
}

#endif