#ifndef MAKAILIB_EX_GAME_DIALOG_ACTOR_H
#define MAKAILIB_EX_GAME_DIALOG_ACTOR_H

#include <makai/makai.hpp>

#include "box.hpp"

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief Dialog actor.
	struct Actor: IVisible, IPerformer {
		/// @brief Dialog box.
		Instance<Box>	dialog;

		/// @brief Constructs the actor.
		/// @param dialog Dialog box to assign.
		Actor(Instance<Box> const& dialog = nullptr): dialog(dialog) {
			if (dialog) dialog->hide();
		}

		/// @brief Destructor.
		virtual ~Actor() {}
		
		/// @brief Shows the actor.
		void show() override {				}
		/// @brief Hides the actor.
		void hide() override {stepOut();	}

		/// @brief Enters the scene.
		virtual void enter()	{show();						}
		/// @brief Leaves the scene.
		virtual void leave()	{hide();						}
		/// @brief Steps into focus.
		virtual void stepIn()	{if (dialog) dialog->show();	}
		/// @brief Steps out of focus.
		virtual void stepOut()	{if (dialog) dialog->hide();	}
		
		/// @brief Sets the text body color.
		/// @param color Color to set to.
		void color(Vector4 const& color) override	{if (dialog) dialog->setBodyColor(color);					}

		/// @brief Says a dialog line.
		/// @param line Line to say.
		/// @return Time it takes to say the dialog line.
		usize say(Content const& line) override		{if (dialog) return dialog->setBody(line);		return 0;	}
		/// @brief Adds text to the current dialog line.
		/// @param line Line to say.
		/// @return Time it takes to add text.
		usize add(Content const& line) override		{if (dialog) return dialog->appendBody(line);	return 0;	}

		/// @brief Says a dialog line.
		/// @param line Line to say.
		/// @return Time it takes to say the dialog line.
		virtual usize say(Line const& line)			{if (dialog) return dialog->display(line);		return 0;	}
		/// @brief Adds text to their current dialog line.
		/// @param line Line to say.
		/// @return Time it takes to add text.
		virtual usize add(Line const& line)			{if (dialog) return dialog->append(line);		return 0;	}
		
		/// @brief Performs an action.
		/// @param action Action to perform.
		/// @return Time it takes to perform the action.
		usize perform(Action const& action) override {
			switch (action.name) {
				case (ConstHasher::hash("enter")):		enter();	return 0;
				case (ConstHasher::hash("leave")):		leave();	return 0;
				case (ConstHasher::hash("step-in")):	stepIn();	return 0;
				case (ConstHasher::hash("step-out")):	stepOut();	return 0;
			}
			return 0;
		}

		/// @brief Emotes an emotion.
		/// @param emotion Emotion to emote.
		/// @return Time it takes to emote the emotion.
		virtual usize emote(Emotion const& emotion) override {
			return 0;
		}
	};

	/// @brief Actor reference wrapper. Does everything an actor does, while accounting for null references.
	struct ActorRef {
		/// @brief Handle to actor.
		Handle<Actor> actor;
		void color(Vector4 const& color)	{if (actor) actor->color(color);				}
		usize say(Content const& line)		{if (actor) return actor->say(line); return 0;	}
		usize add(Content const& line)		{if (actor) return actor->add(line); return 0;	}
		usize say(Line const& line)			{if (actor) return actor->say(line); return 0;	}
		usize add(Line const& line)			{if (actor) return actor->add(line); return 0;	}
		usize perform(Action const& action) {
			if (actor)
				return actor->perform(action);
			return 0;
		}
		usize emote(Emotion const& emotion) {
			if (actor)
				return actor->emote(emotion);
			return 0;
		}
	};
}

#endif