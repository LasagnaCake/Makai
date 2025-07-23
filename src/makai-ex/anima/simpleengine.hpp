#ifndef MAKAILIB_EX_ANIMA_SIMPLEENGINE_H
#define MAKAILIB_EX_ANIMA_SIMPLEENGINE_H

#include "engine.hpp"

namespace Makai::Ex::AVM {
	/// @brief Simplified anima engine. Comes with (most) bells & whistles.
	struct SimpleEngine: private Engine, IPlayable {
		using Engine::state, Engine::error;

		using
			typename Engine::State,
			typename Engine::Parameters,
			typename Engine::ActiveCast
		;

		/// @brief Constructs the dialog player.
		/// @param scene Scene to use.
		SimpleEngine(): Engine() {
		}

		/// @brief Constructs the dialog player.
		/// @param binpath Path to dialog program.
		/// @param scene Scene to use.
		SimpleEngine(String const& binpath): SimpleEngine() {
			setProgram(binpath);
		}

		/// @brief Sets the dialog program to use.
		/// @param binpath Path to dialog program.
		/// @return Reference to self.
		/// @note Stops the engine, if running.
		SimpleEngine& setProgram(String const& binpath) {
			setProgram(AVM::Anima::fromBytes(File::getBinary(binpath)));
			return *this;
		}

		/// @brief Sets the dialog program to use.
		/// @param diag Dialog program to use.
		/// @return Reference to self.
		/// @note Stops the engine, if running.
		SimpleEngine& setProgram(AVM::Anima const& diag) {
			stop();
			Engine::setProgram(diag);
			return *this;
		}

		/// @brief Starts the dialog.
		/// @return Reference to self. 
		SimpleEngine& start() override final {
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
		SimpleEngine& stop() override final		{isFinished = true; return *this; endProgram();	}
		/// @brief Unpauses the dialog.
		/// @return Reference to self.
		SimpleEngine& play() override final		{paused = false; return *this;					}
		/// @brief Pauses the dialog.
		/// @return Reference to self.
		SimpleEngine& pause() override final	{paused = true; return *this;					}

	protected:
		/// @brief Behaviour to execute when a "back" choice (-1) is selected.
		enum class OnBackBehaviour: usize {
			OBB_DO_NOTHING		= 0,
			OBB_TERMINATE		= ConstHasher::hash("terminate"),
			OBB_EXIT_BLOCK		= ConstHasher::hash("exit-block"),
			OBB_ERROR			= ConstHasher::hash("error"),
			OBB_REPEAT_BLOCK	= ConstHasher::hash("repeat-block"),
		};

		/// @brief Advances the engine forward.
		void process() {
			if (
				state() == State::AVM_ES_FINISHED
			||	state() == State::AVM_ES_READY
			) return;
			if (state() != State::AVM_ES_RUNNING) {
				stop();
				return;
			}
			if (needsChoice && !hasChoice) return;
			needsChoice	= false;
			if (isFinished || paused) return;
			if (hasChoice && getInt() == -1) {
				switch (onBack) {
					case OnBackBehaviour::OBB_EXIT_BLOCK:	forceBlockExit();									break;
					case OnBackBehaviour::OBB_TERMINATE:	stop();												break;
					case OnBackBehaviour::OBB_ERROR:		setErrorAndStop(ErrorCode::AVM_EEC_INVALID_VALUE);	break;
					case OnBackBehaviour::OBB_REPEAT_BLOCK:	jumpToBlockStart();									break;
					default: break;
				}
			}
			clearChoice();
			advanceCounters();
			if (shouldProcess()) next();
		}

		/// @brief Returns a string as a behaviour.
		/// @param name String to convert.
		/// @return String as behaviour.
		constexpr static OnBackBehaviour getBehaviourByName(String const& name) {
			return static_cast<OnBackBehaviour>(ConstHasher::hash(name));
		}

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

		/// @brief Exeutes a global action by a name hash.
		/// @param name Function to call.
		/// @param values Values to pass to it.
		/// @return Whether action was found.
		virtual bool execute(usize const name, Parameters const& params) {
			if (params.size())
			switch (name) {
				case (ConstHasher::hash("on-back")):	onBack		= getBehaviourByName(params.front());	return true;
				case (ConstHasher::hash("autoplay")):	autoplay	= toBool(params.front());				return true;
				case (ConstHasher::hash("delay")):		delay		= toUInt64(params.front());				return true;
			}
			return false;
		}

		/// @brief Called when a global value is requested.
		/// @param name Global to get.
		/// @return Value.
		virtual ssize onGlobalInt(usize const name)				{return 0;	}

		/// @brief Called when a global value is requested.
		/// @param name Global to get.
		/// @return Value.
		virtual String onGlobalString(usize const name)			{return "";	}

		/// @brief Called when a choice is requested.
		/// @param choices Choices to make.
		/// @return Chosen choice.
		virtual void onChoice(Parameters const& choices)						{			}

		/// @brief Called when a scene dialog line is requested to be said.
		/// @param line Line to say.
		/// @return Time it takes to say the line.
		virtual usize onSay(String const& line)									{return 0;	}
		/// @brief Called when a scene dialog line is requested to be added.
		/// @param line Line to add.
		/// @return Time it takes to add the line.
		virtual usize onAdd(String const& line)									{return 0;	}
		/// @brief Called when a scene emotion is requested to be emoted.
		/// @param line Emotion to emote.
		/// @return Time it takes to emote the emotion.
		virtual usize onEmote(uint64 const emotion)								{return 0;	}
		/// @brief Called when a scene action is requested to be performed.
		/// @param action Action to perform.
		/// @param params Action parameters.
		/// @return Time it takes to perform the action.
		virtual usize onPerform(uint64 const action, Parameters const& params)	{return 0;	}
		/// @brief Called when a scene text color change is requested.
		/// @param color Color to set.
		virtual void onTextColor(Vector4 const& color)							{			}
		
		/// @brief Called when actors are requested to say a line.
		/// @param actors Requested actors.
		/// @param line Line to say.
		/// @return Time it takes to say the line.
		virtual usize onActorSay(ActiveCast const& actors, String const& line)									{return 0;	}
		/// @brief Called when actors are requested to add a line.
		/// @param actors Requested actors.
		/// @param line Line to add.
		/// @return Time it takes to say the line.
		virtual usize onActorAdd(ActiveCast const& actors, String const& line)									{return 0;	}
		/// @brief Called when actors are requested to emote.
		/// @param actors Requested actors.
		/// @param emotion Emotion to emote.
		/// @return Time it takes to emote.
		virtual usize onActorEmote(ActiveCast const& actors, uint64 const emotion)								{return 0;	}
		/// @brief Called when actors are requested to perform.
		/// @param actors Requested actors.
		/// @param Action Action to perform.
		/// @param params Action parameters.
		/// @return Time it takes to perform.
		virtual usize onActorPerform(ActiveCast const& actors, uint64 const action, Parameters const& params)	{return 0;	}
		/// @brief Called when actors are requested to change their text color.
		/// @param actors Requested actors.
		/// @param color Color to set.
		virtual void onActorTextColor(ActiveCast const& actors, Vector4 const& color)							{			}

		/// @brief Advance type.
		enum class AdvanceType {
			APAT_USER_INPUT,
			APAT_AUTO_ADVANCE,
			APAT_SYNC
		};

		/// @brief Called when the engine advances from a sync or an user input.
		/// @param advance Advance type.
		virtual void onAdvance(AdvanceType const advance)	{DEBUGLN("Dialog advance: ", enumcast(advance));}

		/// @brief Returns whether the user has advanced the dialog.
		/// @returns Whether user advanced the dialog.
		virtual bool userHasAdvanced() {return true;}

		/// @brief Sets the current choice.
		/// @param choice Choice.
		void setChoice(ssize const choice) {
			setCurrentInt(choice);
			postChoice();
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
		usize	delay			= 600;
		/// @brief Whether to wait for user input.
		bool	waitForUser		= false;
		/// @brief Whether autoplay is enabled.
		bool	autoplay		= false;
		/// @brief What to do on a "back" choice (-1).
		OnBackBehaviour	onBack	= OnBackBehaviour::OBB_DO_NOTHING;

	private:
		/// @brief Whether the engine is waiting for actions to finish processing.
		bool	inSync			= false;
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

		bool shouldProcess() {
			if (syncing()) return false;
			if (!autoplay && waitForUser && userAdvanced())	return true;
			if (!waiting())									return true;
			if (!waitForUser)								return true;
			return false;
		}

		SimpleEngine& next() {
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
			return (!waiting()) || (userHasAdvanced());
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