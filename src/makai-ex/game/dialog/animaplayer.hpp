#ifndef MAKAILIB_EX_GAME_DIALOG_ANIMAPLAYER_H
#define MAKAILIB_EX_GAME_DIALOG_ANIMAPLAYER_H

#include <makai/makai.hpp>

#include "../../anima/anima.hpp"
#include "../core/core.hpp"

// TODO:
//	- Separate this engine code into a separate class (so it can be reused)
//	- - Make sure (user wait, autoplay) can be changed from C++ as well

/// @brief Dialog facilities.
namespace Makai::Ex::Game::Dialog {
	/// @brief Anima-based dialog player.
	struct AAnimaPlayer: private AVM::Engine, IPlayable, AUpdateable, Controllable {
		using Engine::state, Engine::error;

		using
			typename Engine::State,
			typename Engine::Parameters,
			typename Engine::ActiveCast
		;

		/// @brief Constructs the dialog player.
		/// @param scene Scene to use.
		AAnimaPlayer(): AVM::Engine() {
			bindmap = Dictionary<String>({	
				{"next", "dialog/next"},
				{"skip", "dialog/skip"}
			});
		}

		/// @brief Constructs the dialog player.
		/// @param binpath Path to dialog program.
		/// @param scene Scene to use.
		AAnimaPlayer(String const& binpath): AAnimaPlayer() {
			setProgram(binpath);
		}

		/// @brief Sets the dialog program to use.
		/// @param binpath Path to dialog program.
		/// @return Reference to self.
		/// @note Stops the engine, if running.
		AAnimaPlayer& setProgram(String const& binpath) {
			setProgram(AVM::Anima::fromBytes(File::getBinary(binpath)));
			return *this;
		}

		/// @brief Sets the dialog program to use.
		/// @param diag Dialog program to use.
		/// @return Reference to self.
		/// @note Stops the engine, if running.
		AAnimaPlayer& setProgram(AVM::Anima const& diag) {
			stop();
			Engine::setProgram(diag);
			return *this;
		}

		/// @brief Executed every update cycle.
		void onUpdate(float, Makai::App&) override {
			if (
				state() == State::AVM_ES_FINISHED
			||	state() == State::AVM_ES_READY
			) return;
			if (state() != State::AVM_ES_RUNNING) {
				stop();
				return;
			}
			if (needsChoice && !hasChoice) return;
			needsChoice = false;
			if (isFinished || paused) return;
			advanceCounters();
			if (shouldProcess()) next();
		}

		/// @brief Starts the dialog.
		/// @return Reference to self. 
		AAnimaPlayer& start() override final {
			isFinished	=
			inSync		=
			autoplay	=
			waitForUser	= false;
			actionDelay = 0;
			resetCounters();
			beginProgram();
			return play();
		}

		/// @brief Stops the dialog.
		/// @return Reference to self.
		AAnimaPlayer& stop() override final		{isFinished = true; return *this; endProgram();	}
		/// @brief Unpauses the dialog.
		/// @return Reference to self.
		AAnimaPlayer& play() override final		{paused = false; return *this;					}
		/// @brief Pauses the dialog.
		/// @return Reference to self.
		AAnimaPlayer& pause() override final	{paused = true; return *this;					}

	protected:
		/// @brief Returns a color by a name hash.
		/// @param name Name hash.
		/// @return Color.
		virtual Vector4 getColorByName(uint64 const name) {
			switch (name) {
				case (ConstHasher::hash("white")):		return Graph::Color::WHITE;
				case (ConstHasher::hash("gray")):		return Graph::Color::GRAY;
				case (ConstHasher::hash("black")):		return Graph::Color::BLACK;
				case (ConstHasher::hash("red")):		return Graph::Color::RED;
				case (ConstHasher::hash("yellow")):		return Graph::Color::YELLOW;
				case (ConstHasher::hash("green")):		return Graph::Color::GREEN;
				case (ConstHasher::hash("cyan")):		return Graph::Color::CYAN;
				case (ConstHasher::hash("blue")):		return Graph::Color::BLUE;
				case (ConstHasher::hash("magenta")):	return Graph::Color::MAGENTA;
			}
			return Graph::Color::WHITE;
		}

		/// @brief Calls a global function by a name hash.
		/// @param name Function to call.
		/// @param values Values to pass to it.
		virtual void execute(usize const name, Parameters const& params) {
			switch (name) {
				case (ConstHasher::hash("autoplay")):	autoplay	= toBool(params[0]);	break;
				case (ConstHasher::hash("delay")):		delay		= toUInt64(params[0]);	break;
			}
		}

		/// @brief Called when a global value is requested. Must be implemented.
		/// @param name Global to get.
		/// @return Value.
		virtual ssize onGlobalInt(usize const name)				{return 0;	}

		/// @brief Called when a global value is requested. Must be implemented.
		/// @param name Global to get.
		/// @return Value.
		virtual String onGlobalString(usize const name)			{return "";	}

		/// @brief Called when a choice is requested. Must be implemented.
		/// @param choices Choices to make.
		/// @return Chosen choice.
		virtual void onChoice(Parameters const& choices)	= 0;

		/// @brief Called when a scene dialog line is requested to be said. Must be implemented.
		/// @param line Line to say.
		/// @return Time it takes to say the line.
		virtual usize onSay(String const& line)									= 0;
		/// @brief Called when a scene dialog line is requested to be added. Must be implemented.
		/// @param line Line to add.
		/// @return Time it takes to add the line.
		virtual usize onAdd(String const& line)									= 0;
		/// @brief Called when a scene emotion is requested to be emoted. Must be implemented.
		/// @param line Emotion to emote.
		/// @return Time it takes to emote the emotion.
		virtual usize onEmote(uint64 const emotion)								= 0;
		/// @brief Called when a scene action is requested to be performed. Must be implemented.
		/// @param action Action to perform.
		/// @param params Action parameters.
		/// @return Time it takes to perform the action.
		virtual usize onPerform(uint64 const action, Parameters const& params)	= 0;
		/// @brief Called when a scene text color change is requested. Must be implemented.
		/// @param color Color to set.
		virtual void onTextColor(Vector4 const& color)							= 0;
		
		/// @brief Called when actors are requested to say a line. Must be implemented.
		/// @param actors Requested actors.
		/// @param line Line to say.
		/// @return Time it takes to say the line.
		virtual usize onActorSay(ActiveCast const& actors, String const& line)									= 0;
		/// @brief Called when actors are requested to add a line. Must be implemented.
		/// @param actors Requested actors.
		/// @param line Line to add.
		/// @return Time it takes to say the line.
		virtual usize onActorAdd(ActiveCast const& actors, String const& line)									= 0;
		/// @brief Called when actors are requested to emote. Must be implemented.
		/// @param actors Requested actors.
		/// @param emotion Emotion to emote.
		/// @return Time it takes to emote.
		virtual usize onActorEmote(ActiveCast const& actors, uint64 const emotion)								= 0;
		/// @brief Called when actors are requested to perform. Must be implemented.
		/// @param actors Requested actors.
		/// @param Action Action to perform.
		/// @param params Action parameters.
		/// @return Time it takes to perform.
		virtual usize onActorPerform(ActiveCast const& actors, uint64 const action, Parameters const& params)	= 0;
		/// @brief Called when actors are requested to change their text color. Must be implemented.
		/// @param actors Requested actors.
		/// @param color Color to set.
		virtual void onActorTextColor(ActiveCast const& actors, Vector4 const& color)							= 0;

		enum class AdvanceType {
			APAT_USER_INPUT,
			APAT_AUTO_ADVANCE,
			APAT_SYNC
		};

		/// @brief Called when the engine advances from a sync or an user input.
		/// @param advance Advance type.
		virtual void onAdvance(AdvanceType const advance)	{DEBUGLN("Dialog advance: ", enumcast(advance));}

		/// @brief Sets the current choice.
		/// @param choice Choice.
		void setChoice(ssize const choice) {
			setCurrentInt(choice);
			postChoice();
		}

		bool shouldProcess() {
			if (syncing()) return false;
			if (!autoplay && waitForUser && userAdvanced())	return true;
			if (!waiting())									return true;
			if (!waitForUser)								return true;
			return false;
		}

		/// @brief Tells the dialog player that the user has made a choice.
		void postChoice()	{hasChoice = true;	}
		/// @brief Clears the current choice.
		void clearChoice()	{hasChoice = false;	}

		/// @brief Sets the AVM's current string value.
		/// @param value Value to set.
		void setCurrentString(String const& value)	{setString(value);	}
		/// @brief Sets the AVM's current integer value.
		/// @param value Value to set.
		void setCurrentInt(ssize const value)		{setInt(value);		}

		/// @brief Max time to wait for user input.
		usize delay = 600;

	private:
		/// @brief Whether the engine is waiting for actions to finish processing.
		bool	inSync			= false;
		/// @brief Whether autoplay is enabled.
		bool	autoplay		= false;
		/// @brief Whether to wait for user input.
		bool	waitForUser		= false;
		/// @brief Whether the user has chosen something.
		bool	hasChoice		= false;
		/// @brief Whether a choice was requested.
		bool	needsChoice		= false;
		/// @brief Counter for the auto-advance timer.
		usize	autoCounter		= 0;
		/// @brief Counter for the action sync timer.
		usize	actionCounter	= 0;
		/// @brief Time to wait for actions to finish processing.
		usize	actionDelay		= 0;

		AAnimaPlayer& next() {
			if (isFinished) return *this;
			if (inSync)				onAdvance(AdvanceType::APAT_SYNC);
			else if (waitForUser)	onAdvance(AdvanceType::APAT_USER_INPUT);
			else if (!waiting())	onAdvance(AdvanceType::APAT_AUTO_ADVANCE);
			if (inSync) clearActionDelay();
			inSync		=
			waitForUser	= false;
			resetCounters();
			do {
				Engine::process();
				if (state() != Engine::State::AVM_ES_RUNNING)
					isFinished = true;
			} while (!(isFinished || waitForUser || inSync || needsChoice));
			return *this;
		}

		void opSay(ActiveCast const& actors, String const& line) override final {
			if (!actors.exclude && actors.actors.empty())
				setActionDelay(onSay(line));
			else setActionDelay(onActorSay(actors, line));
		}

		void opAdd(ActiveCast const& actors, String const& line) override final {
			if (!actors.exclude && actors.actors.empty())
				setActionDelay(onAdd(line));
			else setActionDelay(onActorAdd(actors, line));
		}

		void opEmote(ActiveCast const& actors, uint64 const emotion) override final {
			if (!actors.exclude && actors.actors.empty())
				setActionDelay(onEmote(emotion));
			else setActionDelay(onActorEmote(actors, emotion));
		}

		void opPerform(ActiveCast const& actors, uint64 const action, Parameters const& params) override final {
			if (!actors.exclude && actors.actors.empty())
				setActionDelay(onPerform(action, params));
			else setActionDelay(onActorPerform(actors, action, params));
		}

		void opColor(ActiveCast const& actors, uint64 const color) override final {
			if (!actors.exclude && actors.actors.empty())
				onTextColor(Graph::Color::fromHexCodeRGBA(color));
			else onActorTextColor(actors, Graph::Color::fromHexCodeRGBA(color));
		}

		void opColorRef(ActiveCast const& actors, uint64 const color) override final {
			if (!actors.exclude && actors.actors.empty())
				onTextColor(getColorByName(color));
			else onActorTextColor(actors, getColorByName(color));
		}

		void opDelay(uint64 const time) override final {
			actionDelay = time;
		}

		void opWaitForActions(bool const async) override final {
			inSync = true;
		}

		void opWaitForUser() override final {
			waitForUser = true;
		}

		void opNamedCallSingle(uint64 const name, String const& param) override final {
			execute(name, Parameters{param});
		}

		void opNamedCallMultiple(uint64 const name, Parameters const& params) override final {
			execute(name, params);
		}

		void opGetInt(uint64 const name) override final {
			setInt(onGlobalInt(name));
		}

		void opGetString(uint64 const name) override final {
			setString(onGlobalString(name));
		}

		void opGetChoice(uint64 const name, Parameters const& choices) override final {
			DEBUGLN("Choice needed!");
			clearChoice();
			needsChoice = true;
			onChoice(choices);
		}

		void resetCounters() {
			autoCounter		= 0;
			actionCounter	= 0;
		}

		void clearActionDelay() {
			actionDelay = 0;
		}

		void advanceCounters() {
			++autoCounter;
			++actionCounter;
		}

		bool userAdvanced() {
			return (!waiting()) || (
				action("next", true)
			||	action("skip")
			);
		}

		bool waiting() {
			return autoCounter < delay;
		}

		bool syncing() {
			return inSync && actionCounter < actionDelay;
		}

		void setActionDelay(uint64 const time) {
			if (actionDelay < time) actionDelay = time;
		}
	};
}

#endif
