#ifndef MAKAILIB_EX_GAME_DIALOG_BOX_H
#define MAKAILIB_EX_GAME_DIALOG_BOX_H

#include <makai/makai.hpp>

#include "core.hpp"

namespace Makai::Ex::Game::Dialog {
	struct Box: IVisible {
		Graph::Label	title;
		Graph::Label	body;

		struct Layers {
			usize title;
			usize body	= title;
		};

		virtual ~Box() {}

		virtual void setRenderLayers(Layers const& layers) {
			title.setRenderLayer(layers.title);
			body.setRenderLayer(layers.body);
		}

		void show() override {title.active = body.active = true;	}
		void hide() override {title.active = body.active = false;	}

		void setTitleColor(Vector4 const& color)	{title.material.color = color;	}
		void setBodyColor(Vector4 const& color)		{body.material.color = color;	}

		virtual void setColor(Vector4 const& color) {
			setTitleColor(color);
			setBodyColor(color);
		}

		virtual void setTitle(Content const& line) {
			title.text->content = line.content;
			setTitleColor(line.color);
		}

		virtual void setBody(Content const& line) {
			body.text->content = line.content;
			setBodyColor(line.color);
		}

		virtual void appendTitle(Content const& line) {
			title.text->content += line.content;
			setTitleColor(line.color);
		}

		virtual void appendBody(Content const& line) {
			body.text->content += line.content;
			setBodyColor(line.color);
		}

		virtual void display(Line const& line) {
			setTitle(line.title);
			setBody(line.body);
		}

		virtual void append(Line const& line) {
			appendTitle(line.title);
			appendBody(line.body);
		}
	};
}

#endif