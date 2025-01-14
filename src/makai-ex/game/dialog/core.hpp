#ifndef MAKAILIB_EX_GAME_DIALOG_CORE_H
#define MAKAILIB_EX_GAME_DIALOG_CORE_H

#include <makai/makai.hpp>

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	using Graphic = Instance<Graph::AGraphic>;

	/// @brief Dialog box text content.
	struct Content {
		/// @brief Text content.
		String	content;
		/// @brief Text color.
		Vector4	color	= Graph::Color::WHITE;
	};

	/// @brief Dialog box line.
	struct Line {
		/// @brief Box body title.
		Content	title;
		/// @brief Box body text.
		Content	body;
	};
	
	/// @brief Performer method.
	struct Method {
		/// @brief Method name.
		usize		name;
		/// @brief Method parameters.
		StringList	params;
	};
	
	/// @brief Performer action.
	struct Action:	Method	{};
	/// @brief Performer emotion.
	struct Emotion:	Method	{};

	/// @brief Performer object interface.
	struct IPerformer {
		/// @brief Destructor.
		virtual ~IPerformer() {}

		/// @brief Sets the text color. Must be implemented.
		/// @param color Color to set text to.
		virtual void color(Vector4 const& color)	= 0;
		/// @brief Says a dialog line. Must be implemented.
		/// @param line Line to say.
		/// @return Time it takes to say the dialog line.
		virtual usize say(Content const& line)		= 0;
		/// @brief Adds text to the current dialog line. Must be implemented.
		/// @param line Line to add.
		/// @return Time it takes to add the dialog line.
		virtual usize add(Content const& line)		= 0;
		/// @brief Performs an action. Must be implemented.
		/// @param action Action to perform.
		/// @return Time it takes to perform the action.
		virtual usize perform(Action const& action)	= 0;
		/// @brief Emotes an emotion. Must be implemented.
		/// @param emotion Emotion to emote.
		/// @return Time it takes to emote the emotion.
		virtual usize emote(Emotion const& emotion)	= 0;
	};
}

#endif