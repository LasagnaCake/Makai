#ifndef MAKAILIB_EX_GAME_DIALOG_DVM_ENGINE_H
#define MAKAILIB_EX_GAME_DIALOG_DVM_ENGINE_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

/// @brief Dialog Virtual Machine.
namespace Makai::Ex::Game::Dialog::DVM {
	/// @brief Dialog engine.
	struct Engine {
		/// @brief Function parameters.
		using Parameters = StringList;

		/// @brief Engine state.
		enum class State {
			DVM_ES_READY,
			DVM_ES_RUNNING,
			DVM_ES_ERROR,
			DVM_ES_FINISHED,
		};

		/// @brief Engine error code.
		enum class ErrorCode {
			DVM_EEC_NONE,
			DVM_EEC_INVALID_OPERATION,
			DVM_EEC_INVALID_OPERAND,
			DVM_EEC_INVALID_JUMP,
		};

		/// @brief Destructor.
		virtual ~Engine() {}

		/// @brief Processes one dialog operation.
		void process() {
			if (engineState != State::DVM_ES_RUNNING) return;
			if (op >= binary.code.size()) return opHalt();
			curOp = binary.code[op++];
			switch (asOperation(curOp)) {
				case (Operation::DVM_O_NO_OP):		opSetSP();		break;
				case (Operation::DVM_O_HALT):		opHalt();		break;
				case (Operation::DVM_O_ACTOR):		opActor();		break;
				case (Operation::DVM_O_LINE):		opLine();		break;
				case (Operation::DVM_O_EMOTION):	opEmotion();	break;
				case (Operation::DVM_O_ACTION):		opAction();		break;
				case (Operation::DVM_O_COLOR):		opColor();		break;
				case (Operation::DVM_O_WAIT):		opWait();		break;
				case (Operation::DVM_O_SYNC):		opSync();		break;
				case (Operation::DVM_O_USER_INPUT):	opUserInput();	break;
				case (Operation::DVM_O_NAMED_CALL):	opNamedCall();	break;
				case (Operation::DVM_O_JUMP):		opJump();		break;
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
		/// @brief Named global operation.
		/// @param param Global name.
		/// @param value Value to pass.
		virtual void opNamedCallSingle(uint64 const param, String const& value)							{}
		/// @brief Named global operation.
		/// @param param Global name.
		/// @param values Values to pass.
		virtual void opNamedCallMultiple(uint64 const param, Parameters const& values)					{}

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
			engineState = State::DVM_ES_READY;
		}

		/// @brief Starts the processing of the dialog.
		void beginProgram() {
			engineState	= State::DVM_ES_RUNNING;
			op			= 0;
		}

		/// @brief Stops the processing of the dialog.
		void endProgram() {
			if (engineState == State::DVM_ES_RUNNING)
				engineState = State::DVM_ES_FINISHED;
		}

	protected:
		/// @brief Sets the error code and stops execution.
		/// @param code Error code to set.
		void setErrorAndStop(ErrorCode const code) {
			err = code;
			engineState = State::DVM_ES_ERROR;
		}

	private:
		/// @brief Dialog being processed.
		Dialog binary;

		/// @brief Actors being operated on.
		ActiveCast	actors;
		/// @brief SP mode being used.
		uint16		spMode		= 0;
		/// @brief Engine state.
		State		engineState	= State::DVM_ES_READY;
		/// @brief Operation pointer.
		usize		op			= 0;
		/// @brief Error code.
		ErrorCode	err			= ErrorCode::DVM_EEC_NONE;
		/// @brief Current operation.
		uint16		curOp		= 0;

		uint16 sp() {
			auto sm = spMode;
			if (!sm) sm = getSPFlag(curOp);
			spMode = 0;
			return spMode;
		}

		void opInvalidOp() {
			setErrorAndStop(ErrorCode::DVM_EEC_INVALID_OPERATION);
		}

		void opHalt() {
			engineState = State::DVM_ES_FINISHED;
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
			if (!sp()) return opPerform(actors, action, StringList());
			uint64 params, psize;
			if (!operands64(params, psize)) return;
			if (psize)
				opPerform(actors, action, binary.data.sliced(params, params + psize));
			else
				opPerform(actors, action, StringList());
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

		void opNamedCall() {
			uint64 param, value;
			if (!operands64(param, value)) return;
			if (!sp()) return opNamedCallSingle(param, binary.data[value]);
			uint64 vcount;
			if (!operand64(vcount)) return;
			if (vcount)
				opNamedCallMultiple(param, binary.data.sliced(value, value + vcount));
			else
				opNamedCallMultiple(param, StringList());
		}

		void opJump() {
			uint64 to;
			if (!operand64(to)) return;
			if (!binary.jumps.contains(to))
				return setErrorAndStop(ErrorCode::DVM_EEC_INVALID_JUMP);
			op = binary.jumps[to];
			if (op >= binary.code.size())
				return setErrorAndStop(ErrorCode::DVM_EEC_INVALID_JUMP);
		}

		constexpr bool assertOperand(usize const opsize) {
			if (op + opsize < binary.code.size()) {
				setErrorAndStop(ErrorCode::DVM_EEC_INVALID_OPERAND);
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