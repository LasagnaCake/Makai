#ifndef MAKAILIB_EX_GAME_DIALOG_BOX_H
#define MAKAILIB_EX_GAME_DIALOG_BOX_H

#include <makai/makai.hpp>

#include "core.hpp"

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief Dialog box.
	struct Box: IVisible {
		/// @brief Dialog box title.
		Graph::Label	title;
		/// @brief Dialog box text body.
		Graph::Label	body;

		/// @brief Dialog box render layers.
		struct Layers {
			/// @brief Title layer.
			usize title;
			/// @brief Body layer.
			usize body	= title;
		};

		/// @brief Destructor.
		virtual ~Box() {}

		/// @brief Sets the dialog box's render layers.
		/// @param layers Layers to set box to.
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

		virtual usize setBody(Content const& line) {
			body.text->content = line.content;
			setBodyColor(line.color);
			return 0;
		}

		virtual void appendTitle(Content const& line) {
			title.text->content += line.content;
			setTitleColor(line.color);
		}

		virtual usize appendBody(Content const& line) {
			body.text->content += line.content;
			setBodyColor(line.color);
			return 0;
		}

		virtual usize display(Line const& line) {
			setTitle(line.title);
			return setBody(line.body);
		}

		virtual usize append(Line const& line) {
			appendTitle(line.title);
			return appendBody(line.body);
		}
	};
}

#endif