#ifndef MAKAILIB_EX_GAME_DIALOG_CHOICEMENU_H
#define MAKAILIB_EX_GAME_DIALOG_CHOICEMENU_H

#include <makai/makai.hpp>

#include "../core/core.hpp"

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief Choice menu.
	struct ChoiceMenu: IVisible, AUpdateable, Controllable {
		/// @brief Options display.	
		Graph::Label menu;
		/// @brief Selection cursor display.
		Graph::Label cursor;

		//// @brief Destructor.
		virtual ~ChoiceMenu() {}

		/// @brief Default constructor.
		ChoiceMenu() {
			bindmap = Dictionary<String>({	
				{"next",		"dialog/choice/next"		},
				{"previous",	"dialog/choice/previous"	},
				{"select",		"dialog/next"				},
				{"cancel",		"dialog/skip"				}
			});
		}

		/// @brief Gets called every execution cycle.
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

		/// @brief Sets the curently-highlighted choice.
		/// @param newChoice Choice to select.
		void setChoice(ssize const newChoice) {
			if (prevChoice == newChoice) return;
			choice = newChoice;
			if (options.empty())
				choice = 0;
			else if (choice < 0)
				choice = options.size() - 1;
			else if (static_cast<usize>(choice) > options.size()-1)
				choice = 0;
			onFocusChange(prevChoice, choice);
			prevChoice = choice;
		}
		
		/// @brief Sets the list of choices to display.
		/// @param choices Choice list.
		void setOptions(StringList const& choices) {
			options		= CTL::copy(choices);
			choice		= 0;
			prevChoice	= 0;
			clear();
			onOptionsChanged();	
		}

		/// @brief Returns the current choice list.
		/// @return Current choice list.
		StringList const& getOptions() {
			return options;
		}

		/// @brief Clears the current selection.
		void clear()	{posted = false; cooldown = 0;													}
		/// @brief Selects the currently-highlighted choice.
		void select()	{posted = true; hide();															}
		/// @brief If `exitOnCancel` is true, selects -1. Else, highlights the last choice in the list.
		void cancel()	{if (exitOnCancel) {choice = -1; select();} else setChoice(options.size()-1);	}

		/// @brief Called when the currently-highlighted choice is changed.
		virtual void onFocusChange(ssize const oldChoice, ssize const newChoice) {repaint();}

		/// @brief Called when the list of options is changed.
		virtual void onOptionsChanged()	{repaint();}

		/// @brief Shows & enables the choice menu.
		void show() override {
			menu.active		=
			cursor.active	=
			updating		= true;
			counter			= 0;
		}

		/// @brief Hides & disables the choice menu.
		void hide() override {
			menu.active		=
			cursor.active	=
			updating		= false;
			counter			= 0;
		}

		/// @brief Returns whether a choice is selected.
		bool ready()	{return posted;						}
		/// @brief Returns the currently-selected choice.
		/// @return Currently-selected choice.
		ssize value()	{return choice;						}
		/// @brief Collects & clears the currently-selected choice.
		/// @return Currently-selected choice.
		ssize collect()	{posted = false; return value();	}

		/// @brief Time to wait before starting to respond to user input.
		usize cooldown = 1;

		/// @brief Whether cancelling exits the choice, or highlights the last option.
		bool exitOnCancel = false;

	private:
		/// @brief Counter for timing purposes.
		usize counter = 0;

		/// @brief Whether an option was selected.
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
			setCursor(cursor);
		}

		void setCursor(usize const line) {
			cursor.text->rectAlign.x	= menu.text->rectAlign.x;
			cursor.text->rect.h			= menu.text->rect.h + 4;
			cursor.text->rect.v			= menu.text->rect.v;
			auto& display = cursor.text->content;
			display = String(line, '\n');
			display += "> " + String(cursor.text->rect.h - 4, ' ') + " <";
		}

		/// @brief Options to display.
		StringList options;

		/// @brief Currently-highlighted choice.
		ssize choice		= 0;
		/// @brief Previously-highlighted choice.
		ssize prevChoice	= 0;
	};
}

#endif