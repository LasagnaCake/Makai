#ifndef MAKAILIB_EX_GAME_DIALOG_DVM_ENGINE_H
#define MAKAILIB_EX_GAME_DIALOG_DVM_ENGINE_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

/// @brief Dialog Virtual Machine.
namespace Makai::Ex::Game::Dialog::DVM {
	/// @brief Dialog engine.
	struct Engine {
		/// @brief Function parameters.
		using Parameters = Nullable<StringList>;

		/// @brief Engine state.
		enum class State {
			DSES_READY,
			DSES_RUNNING,
			DSES_ERROR,
			DSES_FINISHED,
		};

		/// @brief Engine error code.
		enum class ErrorCode {
			DSEEC_NONE,
			DSEEC_INVALID_OPERATION,
			DSEEC_INVALID_OPERAND,
			DSEEC_INVALID_JUMP,
		};

		/// @brief Destructor.
		virtual ~Engine() {}

		/// @brief Processes one dialog operation.
		void process() {
			if (engineState != State::DSES_RUNNING) return;
			if (op >= binary.code.size()) return opHalt();
			curOp = binary.code[op++];
			switch (asOperation(curOp)) {
				case (Operation::DSO_NO_OP):		opSetSP();		break;
				case (Operation::DSO_HALT):			opHalt();		break;
				case (Operation::DSO_ACTOR):		opActor();		break;
				case (Operation::DSO_LINE):			opLine();		break;
				case (Operation::DSO_EMOTION):		opEmotion();	break;
				case (Operation::DSO_ACTION):		opAction();		break;
				case (Operation::DSO_COLOR):		opColor();		break;
				case (Operation::DSO_WAIT):			opWait();		break;
				case (Operation::DSO_SYNC):			opSync();		break;
				case (Operation::DSO_USER_INPUT):	opUserInput();	break;
				case (Operation::DSO_SET_GLOBAL):	opSetGlobal();	break;
				case (Operation::DSO_NAMED_OP):		opNamedOp();	break;
				case (Operation::DSO_JUMP):			opJump();		break;
				default:							opInvalidOp();	break;
			}
		}

		/// @brief Cast on which to operate on.
		struct ActiveCast {
			/// @brief Actors to operate.
			Operands64	actors;
			/// @brief Whether the actor list is for excluded actors.
			bool		exclude	= false;
		};

		/// @brief Say operation.
		/// @param actors Actors to operate on.
		/// @param line Line to say.
		virtual void opSay(ActiveCast const& actors, String const& line)								{}
		/// @brief Add operation.
		/// @param actors Actors to operate on.
		/// @param line Line to add.
		virtual void opAdd(ActiveCast const& actors, String const& line)								{}
		/// @brief Emote operation.
		/// @param actors Actors to operate on.
		/// @param emotion Emotion to emote.
		virtual void opEmote(ActiveCast const& actors, uint64 const emotion)							{}
		/// @brief Perform operation.
		/// @param actors Actors to operate on.
		/// @param action Action to perform.
		/// @param params Action parameters.
		virtual void opPerform(ActiveCast const& actors, uint64 const action, Parameters const& params)	{}
		/// @brief Text color operation.
		/// @param actors Actors to operate on.
		/// @param color Hex color to set text to.
		virtual void opColor(ActiveCast const& actors, uint64 const color)								{}
		/// @brief Text color operation.
		/// @param actors Actors to operate on.
		/// @param color Color name to set text to.
		virtual void opColorRef(ActiveCast const& actors, uint64 const color)							{}
		/// @brief Delay operation.
		/// @param time Time to wait.
		virtual void opDelay(uint64 const time)															{}
		/// @brief Synchronization operation.
		/// @param async Whether to wait asynchronously.
		virtual void opWaitForActions(bool const async)													{}
		/// @brief User input operation.
		virtual void opWaitForUser()																	{}
		/// @brief Set global operation.
		/// @param param Global to set.
		/// @param value Value to set to.
		virtual void opSetGlobalValue(uint64 const param, String const& value)							{}
		/// @brief Set global operation.
		/// @param param Global to set.
		/// @param values Values to set to.
		virtual void opSetGlobalValues(uint64 const param, Parameters const& values)					{}
		/// @brief Named operation.
		/// @param name Operation to execute.
		/// @param params Parameters to pass to operation.
		virtual void opNamedOperation(uint64 const name, Parameters const& params)						{}

		/// @brief Returns the error code.
		/// @return Error code.
		constexpr ErrorCode error() const {
			return err;
		}

		/// @brief Returns the engine state.
		/// @return Engine state.
		constexpr State state() const {
			return engineState;
		}

		/// @brief Sets the dialog to process.
		/// @param program Dialog to process.
		void setProgram(Dialog const& program) {
			endProgram();
			binary = program;
			engineState = State::DSES_READY;
		}

		/// @brief Starts the processing of the dialog.
		void beginProgram() {
			engineState	= State::DSES_RUNNING;
			op			= 0;
		}

		/// @brief Stops the processing of the dialog.
		void endProgram() {
			if (engineState == State::DSES_RUNNING)
				engineState = State::DSES_FINISHED;
		}

	protected:
		/// @brief Sets the error code and stops execution.
		/// @param code Error code to set.
		void setErrorAndStop(ErrorCode const code) {
			err = code;
			engineState = State::DSES_ERROR;
		}

	private:
		/// @brief Dialog being processed.
		Dialog binary;

		/// @brief Actors being operated on.
		ActiveCast	actors;
		/// @brief SP mode being used.
		uint16		spMode		= 0;
		/// @brief Engine state.
		State		engineState	= State::DSES_READY;
		/// @brief Operation pointer.
		usize		op			= 0;
		/// @brief Error code.
		ErrorCode	err			= ErrorCode::DSEEC_NONE;
		/// @brief Current operation.
		uint16		curOp		= 0;

		uint16 sp() {
			auto sm = spMode;
			if (!sm) sm = getSPFlag(curOp);
			spMode = 0;
			return spMode;
		}

		void opInvalidOp() {
			setErrorAndStop(ErrorCode::DSEEC_INVALID_OPERATION);
		}

		void opHalt() {
			engineState = State::DSES_FINISHED;
		}

		void opSetSP() {spMode = getSPFlag(curOp);}

		void opActor() {
			uint16 spm	= sp();
			uint16 spco	= getSPFlag(curOp);
			if (spco && spm != spco)
				spm = spco;
			if (spm == 2) {
				actors = {.exclude = true};
				return;		
			}
			uint64 actor;
			if (!operand64(actor)) return;
			if (!actor && !spm)	actors = {};
			else if (spm) {
				if (!actor || spm != 1) return;
				actors.actors.pushBack(actor);
			} else {
				actors = {};
				if (actor) actors.actors.pushBack(actor);
			}
		}

		void opLine() {
			uint64 line;
			if (!operand64(line)) return;
			if (sp() && line)
				opAdd(actors, binary.data[line-1]);
			else opAdd(actors, line ? binary.data[line-1] : "");
		}

		void opEmotion() {
			uint64 emotion;
			if (!operand64(emotion)) return;
			opEmote(actors, emotion);
		}

		void opAction() {
			uint64 action;
			if (!operand64(action)) return;
			if (!sp()) return opPerform(actors, action, nullptr);
			uint64 params, psize;
			if (!operands64(params, psize)) return;
			opPerform(actors, action, psize ? binary.data.sliced(params, params + psize) : nullptr);
		}

		void opColor() {
			uint64 color;
			if (!operand64(color)) return;
			if (sp())	opColorRef(actors, color);
			else		opColor(actors, color);
		}

		void opWait() {
			uint64 frames;
			if (!operand64(frames)) return;
			opDelay(frames);
		}

		void opSync() {
			opWaitForActions(sp());
		}

		void opUserInput() {
			opWaitForUser();
		}

		void opSetGlobal() {
			uint64 param, value;
			if (!operands64(param, value)) return;
			if (!sp()) return opSetGlobalValue(param, binary.data[value]);
			uint64 vcount;
			if (!operand64(vcount)) return;
			opSetGlobalValues(param, vcount ? binary.data.sliced(value, value + vcount) : nullptr);
		}

		void opNamedOp() {
			uint64 name;
			if (!operand64(name)) return;
			if (!sp()) opNamedOperation(name, nullptr);
			uint64 params, psize;
			if (!operands64(params, psize)) return;
			opNamedOperation(name, psize ? binary.data.sliced(params, params + psize) : nullptr);
		}

		void opJump() {
			uint64 to;
			if (!operand64(to)) return;
			if (!binary.jumps.contains(to))
				return setErrorAndStop(ErrorCode::DSEEC_INVALID_JUMP);
			op = binary.jumps[to];
			if (op >= binary.code.size())
				return setErrorAndStop(ErrorCode::DSEEC_INVALID_JUMP);
		}

		constexpr bool assertOperand(usize const opsize) {
			if (op + opsize < binary.code.size()) {
				setErrorAndStop(ErrorCode::DSEEC_INVALID_OPERAND);
				return false;
			}
			return true;
		}

		template<class... Args> constexpr bool operands64(Args&... args) {return (... && operands64(args));}
		template<class... Args> constexpr bool operands32(Args&... args) {return (... && operands32(args));}
		template<class... Args> constexpr bool operands16(Args&... args) {return (... && operands16(args));}

		constexpr bool operand16(uint16& opval) {
			if (!assertOperand(1)) return false;
			MX::memmove(&opval, &binary.code[op], sizeof(uint16));
			op += 1;
			return true;
		}

		constexpr bool operand32(uint32& opval) {
			if (!assertOperand(2)) return false;
			MX::memmove(&opval, &binary.code[op], sizeof(uint32));
			op += 2;
			return true;
		}

		constexpr bool operand64(uint64& opval) {
			if (!assertOperand(4)) return false;
			MX::memmove(&opval, &binary.code[op], sizeof(uint64));
			op += 4;
			return true;
		}
	};
}

#endif