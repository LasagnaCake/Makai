#ifndef MAKAILIB_EX_GAME_DIALOG_CHOICEMENU_H
#define MAKAILIB_EX_GAME_DIALOG_CHOICEMENU_H

#include <makai/makai.hpp>

#include "../core/core.hpp"

namespace Makai::Ex::Game::Dialog {
	struct AChoiceMenu: IVisible, AUpdateable, Controllable {
		Functor<void(ssize const)> onChoice;

		virtual ~AChoiceMenu() {}

		AChoiceMenu() {
			bindmap = Dictionary<String>({	
				{"next",		"dialog/choice/next"		},
				{"previous",	"dialog/choice/previous"	},
				{"select",		"dialog/next"				}
			});
		}

		void onUpdate(float, Makai::App&) override {
			if (!updating) return;
			if (action("select", true))
				select();
			else if (bindmap.contains("cancel") && action("cancel", true))
				cancel();
			else {	
				if (action("next", true))		++choice;
				if (action("previous", true))	--choice;
				setChoice(choice);
			}
		}

		void setChoice(ssize const newChoice) {
			if (prevChoice == newChoice) return;
			choice = newChoice;
			while (choice < 0)					choice += options.size();
			while (choice >= options.size())	choice -= options.size();
			onFocusChange(prevChoice, choice);
			prevChoice = choice;
		}

		void setOptions(StringList const& choices)	{options = choices; onOptionsChanged();	}
		StringList const& getOptions()				{return options;						}

		void select()	{onChoice(choice); hide();				}
		void cancel()	{choice = options.size()-1; select();	}

		virtual void onFocusChange(ssize const oldChoice, ssize const newChoice) 	= 0;
		virtual void onOptionsChanged()												= 0;

	private:
		StringList options;

		ssize choice		= 0;
		ssize prevChoice	= 0;
	};
}

#endif