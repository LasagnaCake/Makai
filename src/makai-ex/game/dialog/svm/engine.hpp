#ifndef MAKAILIB_EX_GAME_DIALOG_SVM_ENGINE_H
#define MAKAILIB_EX_GAME_DIALOG_SVM_ENGINE_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

namespace Makai::Ex::Game::Dialog::SVM {
	struct Engine {
		using Parameters = Nullable<StringList>;

		enum class State {
			DSES_READY,
			DSES_RUNNING,
			DSES_ERROR,
			DSES_FINISHED,
		};

		enum class ErrorCode {
			DSEEC_NONE,
			DSEEC_INVALID_OPERATION,
			DSEEC_INVALID_OPERAND,
			DSEEC_INVALID_JUMP,
		};

		virtual ~Engine() {}

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

		struct ActiveCast {
			Operands64	actors;
			bool		exclude	= false;
		};

		virtual void opSay(ActiveCast const& actors, String const& line)								{}
		virtual void opAdd(ActiveCast const& actors, String const& line)								{}
		virtual void opEmote(ActiveCast const& actors, uint64 const emotion)							{}
		virtual void opPerform(ActiveCast const& actors, uint64 const action, Parameters const& params)	{}
		virtual void opColor(ActiveCast const& actors, uint64 const color)								{}
		virtual void opColorRef(ActiveCast const& actors, uint64 const color)							{}
		virtual void opDelay(uint64 const time)															{}
		virtual void opWaitForActions(bool const async)													{}
		virtual void opWaitForUser()																	{}
		virtual void opSetConfigValue(uint64 const param, String const value)							{}
		virtual void opSetConfigValues(uint64 const param, Parameters const& string)					{}
		virtual void opNamedOperation(uint64 const name, Parameters const& params)						{}

		constexpr ErrorCode error() const {
			return err;
		}

		constexpr State state() const {
			return engineState;
		}

		void setProgram(ByteCode const& program) {
			endProgram();
			binary = program;
			engineState = State::DSES_READY;
		}

		void beginProgram() {
			engineState	= State::DSES_RUNNING;
			op			= 0;
		}

		void endProgram() {
			if (engineState == State::DSES_RUNNING)
				engineState = State::DSES_FINISHED;
		}

	protected:
		void setErrorAndStop(ErrorCode const code) {
			err = code;
			engineState = State::DSES_ERROR;
		}

	private:
		ByteCode binary;

		bool		excludeMode = true;
		ActiveCast	actors;
		uint16		spMode		= 0;
		State		engineState	= State::DSES_READY;
		usize		op			= 0;
		ErrorCode	err			= ErrorCode::DSEEC_NONE;
		uint16		curOp		= 0;

		uint16 sp() {
			auto sm = spMode;
			if (!sm) sm = spFlag(curOp);
			spMode = 0;
			return spMode;
		}

		void opInvalidOp() {
			setErrorAndStop(ErrorCode::DSEEC_INVALID_OPERATION);
		}

		void opHalt() {
			engineState = State::DSES_FINISHED;
		}

		void opSetSP() {spMode = spFlag(curOp);}

		void opActor() {
			uint16 spm	= sp();
			uint16 spco	= spFlag(curOp);
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
			opPerform(actors, action, binary.data.sliced(params, params + psize));
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
			if (!sp()) return opSetConfigValue(param, binary.data[value]);
			uint64 vcount;
			if (!operand64(vcount)) return;
			opSetConfigValues(param, binary.data.sliced(value, value + vcount));
		}

		void opNamedOp() {
			uint64 name;
			if (!operand64(name)) return;
			if (!sp()) opNamedOperation(name, nullptr);
			uint64 params, psize;
			if (!operands64(params, psize)) return;
			opNamedOperation(name, binary.data.sliced(params, params + psize));
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