#ifndef MAKAILIB_EX_ANIMA_ENGINE_H
#define MAKAILIB_EX_ANIMA_ENGINE_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

/// @brief Anima Virtual Machine.
namespace Makai::Ex::AVM {
	/// @brief Anima engine.
	struct Engine {
		/// @brief Function parameters.
		using Parameters = StringList;

		/// @brief Engine state.
		enum class State {
			AVM_ES_READY,
			AVM_ES_RUNNING,
			AVM_ES_ERROR,
			AVM_ES_FINISHED,
		};

		/// @brief Engine error code.
		enum class ErrorCode {
			AVM_EEC_NONE,
			AVM_EEC_INVALID_OPERATION,
			AVM_EEC_INVALID_OPERAND,
			AVM_EEC_INVALID_JUMP,
		};

		/// @brief Destructor.
		virtual ~Engine() {}

		/// @brief Processes one anima operation.
		void process() {
			if (engineState != State::AVM_ES_RUNNING) return;
			if (current.op >= binary.code.size()) return opHalt();
			while (
				current.op < binary.code.size()
			&&	asOperation(curOp = binary.code[current.op++]) == Operation::AVM_O_NEXT
			);
			switch (asOperation(curOp)) {
				case (Operation::AVM_O_NO_OP):		opSetSP();		break;
				case (Operation::AVM_O_HALT):		opHalt();		break;
				case (Operation::AVM_O_ACTOR):		opActor();		break;
				case (Operation::AVM_O_LINE):		opLine();		break;
				case (Operation::AVM_O_EMOTION):	opEmotion();	break;
				case (Operation::AVM_O_ACTION):		opAction();		break;
				case (Operation::AVM_O_COLOR):		opColor();		break;
				case (Operation::AVM_O_WAIT):		opWait();		break;
				case (Operation::AVM_O_SYNC):		opSync();		break;
				case (Operation::AVM_O_USER_INPUT):	opUserInput();	break;
				case (Operation::AVM_O_NAMED_CALL):	opNamedCall();	break;
				case (Operation::AVM_O_JUMP):		opJump();		break;
				case (Operation::AVM_O_GET_VALUE):	opGetValue();	break;
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
		/// @brief Integer value acquisition.
		/// @param name Value name.
		/// @param out Value output.
		virtual void opGetInt(uint64 const name, ssize& out)											{}
		/// @brief String value acquisition.
		/// @param name Value name.
		/// @param out Value output.
		virtual void opGetString(uint64 const name, String& out)										{}
		/// @brief Menu choice acquisition.
		/// @param name Menu name.
		/// @param choices Menu choices.
		/// @param out Value output.
		virtual void opGetChoice(uint64 const name, Parameters const& choices, ssize& out)				{}

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

		/// @brief Returns whether the engine is currently running.
		/// @return Whether engine is currently running.
		constexpr bool running() const {
			return engineState == State::AVM_ES_RUNNING;
		}

		/// @brief Returns whether the engine is currently running.
		/// @return Whether engine is currently running.
		constexpr operator bool() const {return running();}

		/// @brief Sets the anima to process.
		/// @param program Anima to process.
		void setProgram(Anima const& program) {
			endProgram();
			binary = program;
			engineState = State::AVM_ES_READY;
		}

		/// @brief Starts the processing of the anima.
		void beginProgram() {
			engineState	= State::AVM_ES_RUNNING;
			current = Frame();
			current.op = 0;
			stack.clear();
		}

		/// @brief Stops the processing of the anima.
		void endProgram() {
			if (engineState == State::AVM_ES_RUNNING)
				engineState = State::AVM_ES_FINISHED;
			stack.clear();
		}

		/// @brief Random number generator.
		inline static Random::Generator rng;

	protected:
		/// @brief Sets the error code and stops execution.
		/// @param code Error code to set.
		void setErrorAndStop(ErrorCode const code) {
			err = code;
			engineState = State::AVM_ES_ERROR;
		}

		/// @brief Jumps the operation pointer to a named block.
		/// @param name Block to jump to.
		/// @param returnable Whether to return to previous point when block is finished. By default, it is `true`.
		void jumpTo(usize const name, bool const returnable = true) {
			if (returnable)
				storeState();
			if (!binary.jumps.contains(name))
				return setErrorAndStop(ErrorCode::AVM_EEC_INVALID_JUMP);
			current.op = binary.jumps[name];
			if (current.op >= binary.code.size())
				return setErrorAndStop(ErrorCode::AVM_EEC_INVALID_JUMP);
		}

	private:
		/// @brief Anima being processed.
		Anima binary;

		/// @brief Stack frame.
		struct Frame {
			/// @brief Actors being operated on.
			ActiveCast	actors	= {};
			/// @brief SP mode being used.
			uint16		spMode	= 0;
			/// @brief Operation pointer.
			usize		op		= 0;
			/// @brief Current integer.
			ssize		integer	= 0;
			/// @brief Current string.
			String		string	= "";
		};

		/// @brief Program stack.
		List<Frame> stack;
		/// @brief Current execution state.
		Frame		current;

		/// @brief Engine state.
		State		engineState	= State::AVM_ES_READY;
		/// @brief Error code.
		ErrorCode	err			= ErrorCode::AVM_EEC_NONE;
		/// @brief Current operation.
		uint16		curOp		= 0;

		uint16 sp() {
			auto sm = current.spMode;
			if (!sm) sm = getSPFlag(curOp);
			current.spMode = 0;
			return sm;
		}

		void opInvalidOp() {
			setErrorAndStop(ErrorCode::AVM_EEC_INVALID_OPERATION);
		}

		void opHalt() {
			if (sp() && !stack.empty())
				retrieveState();
			else engineState = State::AVM_ES_FINISHED;
		}

		void opSetSP() {current.spMode = getSPFlag(curOp);}

		void opActor() {
			uint16 spm	= sp();
			uint16 spco	= getSPFlag(curOp);
			if (spco && spm != spco)
				spm = spco;
			if (spm == 2) {
				current.actors = {.exclude = true};
				return;		
			}
			uint64 actor;
			if (!operand64(actor)) return;
			if (!actor && !spm)	current.actors = {};
			else if (spm) {
				if (!actor || spm != 1) return;
				current.actors.actors.pushBack(actor);
			} else {
				current.actors = {};
				if (actor) current.actors.actors.pushBack(actor);
			}
		}

		void opLine() {
			uint64 line;
			if (!operand64(line)) return;
			if (sp() && line)
				opAdd(current.actors, binary.data[line]);
			else opSay(current.actors, line ? binary.data[line] : "");
		}

		void opEmotion() {
			uint64 emotion;
			if (!operand64(emotion)) return;
			opEmote(current.actors, emotion);
		}

		void opAction() {
			uint64 action;
			if (!operand64(action)) return;
			if (!sp()) return opPerform(current.actors, action, StringList());
			uint64 params, psize;
			if (!operands64(params, psize)) return;
			if (psize)
				opPerform(current.actors, action, binary.data.sliced(params, params + psize));
			else
				opPerform(current.actors, action, StringList());
		}

		void opColor() {
			uint64 color;
			if (!operand64(color)) return;
			if (sp())	opColorRef(current.actors, color);
			else		opColor(current.actors, color);
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

		void storeState(usize const op) {
			storeState();
			stack.back().op = op;
		}

		void storeState() {
			stack.pushBack(current);
			auto op		= current.op;
			current		= Frame();
			current.op	= op;
		}

		void retrieveState() {
			current	= stack.popBack();
		}

		void opJump() {
			constexpr usize JUMP_SIZE = (sizeof(Operation) + sizeof(uint64)) / 2;
			auto spm = sp();
			if (spm < 2) {
				uint64 to;
				if (!operand64(to)) return;
				return jumpTo(to, spm);
			}
			uint64 range;
			if (!operand64(range)) return;
			if (spm & 0b1000)			storeState(current.op + range * JUMP_SIZE);
			if ((spm & 0b0111) == 2)	current.op += current.integer * JUMP_SIZE;
			else						current.op += rng.integer<usize>(0, range-1) * JUMP_SIZE;
		}

		void opGetValue() {
			uint64 name;
			if (!operand64(name)) return;
			auto spm = sp();
			if (spm == 3) {
				uint64 menu, start, size;
				if (!operands64(menu, start, size)) return;
				opGetChoice(menu, binary.data.sliced(start, start + size), current.integer);
				current.integer = Math::clamp<ssize>(current.integer, 0, size);
				return;
			}
			if (spm == 2) return opGetString(name, current.string);
			uint64 min, max;
			if (spm & 1 && !operands64(min, max)) return;
			opGetInt(name, current.integer);
			if (spm & 1) current.integer = Math::clamp<ssize>(current.integer, min, max);
		}

		constexpr bool assertOperand(usize const opsize) {
			if (current.op + opsize < binary.code.size())
				return true;
			setErrorAndStop(ErrorCode::AVM_EEC_INVALID_OPERAND);
			return false;
		}

		template<class... Args> constexpr bool operands64(Args&... args) {return (... && operand64(args));}
		template<class... Args> constexpr bool operands32(Args&... args) {return (... && operand32(args));}
		template<class... Args> constexpr bool operands16(Args&... args) {return (... && operand16(args));}

		constexpr bool operand16(uint16& opval) {
			if (!assertOperand(1)) return false;
			MX::memmove(&opval, &binary.code[current.op], sizeof(uint16));
			current.op += 1;
			return true;
		}

		constexpr bool operand32(uint32& opval) {
			if (!assertOperand(2)) return false;
			MX::memmove(&opval, &binary.code[current.op], sizeof(uint32));
			current.op += 2;
			return true;
		}

		constexpr bool operand64(uint64& opval) {
			if (!assertOperand(4)) return false;
			MX::memmove(&opval, &binary.code[current.op], sizeof(uint64));
			current.op += 4;
			return true;
		}
	};
}

#endif