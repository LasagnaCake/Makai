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
		};

		virtual ~Engine() {}

		void process() {
			if (engineState != State::DSES_RUNNING) return;
			if (op >= binary.code.size()) opHalt();
			switch (asOperation(binary.code[op++])) {
				case (Operation::DSO_NO_OP):						break;
				case (Operation::DSO_HALT):			opHalt();		break;
				case (Operation::DSO_SP):			opEnableSP();	break;
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

		virtual void opSay(Operands64 const& actors, String const& line)								{}
		virtual void opAdd(Operands64 const& actors, String const& line)								{}
		virtual void opEmote(Operands64 const& actors, uint64 const emotion)							{}
		virtual void opPerform(Operands64 const& actors, uint64 const action, Parameters const& params)	{}
		virtual void opColor(Operands64 const& actors, uint64 const color)								{}
		virtual void opDelay(uint64 const time)															{}
		virtual void opWaitForActions(bool const async)													{}
		virtual void opWaitForUser()																	{}
		virtual void opSetConfigValue(uint64 const param, uint64 const value)							{}
		virtual void opSetConfigString(uint64 const param, String const& string)						{}
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

		bool sp() {
			if (spMode) {
				spMode = false;
				return true;
			}
			return false;
		}

		void opInvalidOp() {
			setErrorAndStop(ErrorCode::DSEEC_INVALID_OPERATION);
		}

		void opHalt() {
			engineState = State::DSES_FINISHED;
		}

		void opEnableSP() {spMode = true;}

		void opActor() {
			uint64 actor;
			if (!operand64(actor)) return;
			if (sp())	actors.pushBack(actor);
			else		actors.clear().pushBack(actor);
		}

		void opLine() {
			uint64 line;
			if (!operand64(line)) return;
			if (sp())	opSay(actors, line);
			else		opAdd(actors, line);
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
			opEmote(actors, color);
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
			if (sp())	opSetConfigString(param, value);
			else		opSetConfigValue(param, value);
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
			op = to;
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
			MX::memcpy(&opval, &binary.code[op], sizeof(uint16));
			op += 1;
			return true;
		}

		constexpr bool operand32(uint32& opval) {
			if (!assertOperand(2)) return false;
			MX::memcpy(&opval, &binary.code[op], sizeof(uint32));
			op += 2;
			return true;
		}

		constexpr bool operand64(uint64& opval) {
			if (!assertOperand(4)) return false;
			MX::memcpy(&opval, &binary.code[op], sizeof(uint64));
			op += 4;
			return true;
		}

		Operands64	actors;
		bool		spMode		= true;
		State		engineState	= State::DSES_READY;
		usize		op			= 0;
		ErrorCode	err			= ErrorCode::DSEEC_NONE;
	};
}

#endif