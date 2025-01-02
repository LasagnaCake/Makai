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

		/// @brief Shows the dialog box.
		void show() override {title.active = body.active = true;	}
		/// @brief Hides the dialog box.
		void hide() override {title.active = body.active = false;	}

		/// @brief Sets the title's text color.
		/// @param color Color to set text to.
		void setTitleColor(Vector4 const& color)	{title.material.color = color;	}
		/// @brief Sets the body's text color.
		/// @param color Color to set text to.
		void setBodyColor(Vector4 const& color)		{body.material.color = color;	}

		/// @brief Sets the dialog box's colors.
		/// @param color Color to set to.
		virtual void setColor(Vector4 const& color) {
			setTitleColor(color);
			setBodyColor(color);
		}

		/// @brief Sets the dialog box's title.
		/// @param line Text to set to.
		virtual void setTitle(Content const& line) {
			title.text->content = line.content;
			setTitleColor(line.color);
		}

		/// @brief Sets the dialog box's body.
		/// @param line Text to set to.
		/// @return Time taken to finish the operation.
		virtual usize setBody(Content const& line) {
			body.text->content = line.content;
			setBodyColor(line.color);
			return 0;
		}

		/// @brief Adds text to the dialog box's title.
		/// @param line Text to add.
		virtual void appendTitle(Content const& line) {
			title.text->content += line.content;
			setTitleColor(line.color);
		}

		/// @brief Adds text to the dialog box's body.
		/// @param line Text to add.
		/// @return Time taken to finish the operation.
		virtual usize appendBody(Content const& line) {
			body.text->content += line.content;
			setBodyColor(line.color);
			return 0;
		}

		/// @brief Sets the dialog box's text.
		/// @param line Text to set to.
		/// @return Time taken to finish the operation.
		virtual usize display(Line const& line) {
			setTitle(line.title);
			return setBody(line.body);
		}

		/// @brief Adds text to the dialog box.
		/// @param line Text to set to.
		/// @return Time taken to finish the operation.
		virtual usize append(Line const& line) {
			appendTitle(line.title);
			return appendBody(line.body);
		}
	};
}

#endif