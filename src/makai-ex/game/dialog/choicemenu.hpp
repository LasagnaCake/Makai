#ifndef MAKAILIB_EX_GAME_DIALOG_CHOICEMENU_H
#define MAKAILIB_EX_GAME_DIALOG_CHOICEMENU_H

#include <makai/makai.hpp>

#include "../core/core.hpp"

namespace Makai::Ex::Game::Dialog {
	struct ChoiceMenu: IVisible, AUpdateable, Controllable {
		Graph::Label menu;
		Graph::Label cursor;

		virtual ~ChoiceMenu() {}

		ChoiceMenu() {
			bindmap = Dictionary<String>({	
				{"next",		"dialog/choice/next"		},
				{"previous",	"dialog/choice/previous"	},
				{"select",		"dialog/next"				},
				{"cancel",		"dialog/skip"				}
			});
		}

		void onUpdate(float, Makai::App&) override {
			if (!updating) return;
			if (counter < cooldown) {
				++counter;
				return;
			}
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
			while (choice < 0)
				choice += options.size();
			while (choice >= static_cast<ssize>(options.size()))
				choice -= options.size();
			onFocusChange(prevChoice, choice);
			prevChoice = choice;
		}

		void setOptions(StringList const& choices) {
			options		= choices;
			choice		= 0;
			prevChoice	= 0;
			clear();
			onOptionsChanged();	
		}

		StringList const& getOptions() {
			return options;
		}

		void clear()	{posted = false; cooldown = 0;	}
		void select()	{posted = true; hide();			}
		void cancel()	{setChoice(options.size()-1);	}

		virtual void onFocusChange(ssize const oldChoice, ssize const newChoice) {
			repaint();
		}

		virtual void onOptionsChanged()	{
			repaint();
		}

		void show() override {
			menu.active		=
			cursor.active	=
			updating		= true;
		}

		void hide() override {
			menu.active		=
			cursor.active	=
			updating		= false;
		}

		bool ready()	{return posted;						}
		ssize value()	{return choice;						}
		ssize collect()	{posted = false; return value();	}

		usize cooldown = 1;

	private:
		usize counter = 0;

		bool posted = false;

		void repaint() {
			auto& display = menu.text->content;
			display.clear();
			ssize i					= 0;
			usize lines				= 0;
			usize cursor			= 0;
			menu.text->rectAlign.x	= 0.5;
			menu.text->rect.h		= 0;
			menu.text->rect.v		= options.size();
			for (String const& option: options) {
				display += option;
				auto const il = option.split('\n');
				for (auto const& l: il)
					if (menu.text->rect.h < l.size())
						menu.text->rect.h = l.size();
				if (i == choice) cursor = lines;
				auto const extras = Regex::count(option, "\n");
				lines += 2 + extras;
				menu.text->rect.v += extras;
				++i;
				if (i < static_cast<ssize>(options.size())) {
					display += "\n\n";
					menu.text->rect.v += 2;
				}
			}
			DEBUGLN("<display>");
			DEBUGLN(display);
			DEBUGLN("</display>");
			setCursor(cursor);
		}

		void setCursor(usize const line) {
			cursor.text->rectAlign.x	= menu.text->rectAlign.x;
			cursor.text->rect.h			= menu.text->rect.h + 4;
			cursor.text->rect.v			= menu.text->rect.v;
			auto& display = cursor.text->content;
			display = String(line, '\n');
			display += "> " + String(cursor.text->rect.h - 4, ' ') + " <";
			DEBUGLN("<display>");
			DEBUGLN(display);
			DEBUGLN("</display>");
		}

		StringList options;

		ssize choice		= 0;
		ssize prevChoice	= 0;
	};
}

#endif