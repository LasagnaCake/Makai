#ifndef MAKAILIB_EX_GAME_DIALOG_ANIMAPLAYER_H
#define MAKAILIB_EX_GAME_DIALOG_ANIMAPLAYER_H

#include <makai/makai.hpp>

#include "../../anima/anima.hpp"
#include "../core/core.hpp"

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief Anima-based dialog player abstract base.
	struct AAnimaPlayer: AVM::SimpleEngine, AUpdateable, Controllable {
		/// @brief Constructs the dialog player.
		/// @param scene Scene to use.
		AAnimaPlayer(): AVM::SimpleEngine() {
			bindmap = Dictionary<String>({	
				{"next", "dialog/next"},
				{"skip", "dialog/skip"}
			});
		}

		/// @brief Constructs the dialog player.
		/// @param binpath Path to dialog program.
		/// @param scene Scene to use.
		AAnimaPlayer(String const& binpath): AVM::SimpleEngine(binpath) {
			bindmap = Dictionary<String>({	
				{"next", "dialog/next"},
				{"skip", "dialog/skip"}
			});
		}

		/// @brief Executed every update cycle.
		void onUpdate(float, Makai::App&) override {
			SimpleEngine::process();
		}

	protected:
		bool userHasAdvanced() override {
			return (
				action("next", true)
			||	action("skip")
			);
		}

		/// @brief Called when a choice is requested.
		/// @param choices Choices to make.
		/// @return Chosen choice.
		void onChoice(Parameters const& choices) override						= 0;

		/// @brief Called when a scene dialog line is requested to be said.
		/// @param line Line to say.
		/// @return Time it takes to say the line.
		usize onSay(String const& line) override								= 0;
		/// @brief Called when a scene dialog line is requested to be added.
		/// @param line Line to add.
		/// @return Time it takes to add the line.
		usize onAdd(String const& line) override								= 0;
		/// @brief Called when a scene emotion is requested to be emoted.
		/// @param line Emotion to emote.
		/// @return Time it takes to emote the emotion.
		usize onEmote(uint64 const emotion) override							= 0;
		/// @brief Called when a scene action is requested to be performed.
		/// @param action Action to perform.
		/// @param params Action parameters.
		/// @return Time it takes to perform the action.
		usize onPerform(uint64 const action, Parameters const& params) override	= 0;
		/// @brief Called when a scene text color change is requested.
		/// @param color Color to set.
		void onTextColor(Vector4 const& color) override							= 0;
		
		/// @brief Called when actors are requested to say a line.
		/// @param actors Requested actors.
		/// @param line Line to say.
		/// @return Time it takes to say the line.
		usize onActorSay(ActiveCast const& actors, String const& line) override									= 0;
		/// @brief Called when actors are requested to add a line.
		/// @param actors Requested actors.
		/// @param line Line to add.
		/// @return Time it takes to say the line.
		usize onActorAdd(ActiveCast const& actors, String const& line) override									= 0;
		/// @brief Called when actors are requested to emote.
		/// @param actors Requested actors.
		/// @param emotion Emotion to emote.
		/// @return Time it takes to emote.
		usize onActorEmote(ActiveCast const& actors, uint64 const emotion) override								= 0;
		/// @brief Called when actors are requested to perform.
		/// @param actors Requested actors.
		/// @param Action Action to perform.
		/// @param params Action parameters.
		/// @return Time it takes to perform.
		usize onActorPerform(ActiveCast const& actors, uint64 const action, Parameters const& params) override	= 0;
		/// @brief Called when actors are requested to change their text color.
		/// @param actors Requested actors.
		/// @param color Color to set.
		void onActorTextColor(ActiveCast const& actors, Vector4 const& color) override							= 0;
	};
}

#endif
