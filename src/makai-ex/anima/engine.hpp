#ifndef MAKAILIB_EX_ANIMA_ENGINE_H
#define MAKAILIB_EX_ANIMA_ENGINE_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

/// @brief Anima Virtual Machine.
namespace Makai::Ex::AVM {
	/// @brief Base anima engine.
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
			AVM_EEC_MISSING_FUNCTION_ARGUMENT,
			AVM_EEC_ARGUMENT_PARSE_FAILURE,
			AVM_EEC_INVALID_VALUE,
			AVM_EEC_IMPLEMENTATION_ERROR,
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
				case (Operation::AVM_O_INVOKE):		opInvoke();		break;
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
		virtual void opGetInt(uint64 const name)														{}
		/// @brief String value acquisition.
		/// @param name Value name.
		virtual void opGetString(uint64 const name)														{}
		/// @brief Choice acquisition.
		/// @param name Choice name.
		/// @param choices Choices.
		virtual void opGetChoice(uint64 const name, Parameters const& choices)							{}

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
			current.op = current.start = binary.jumps[name];
			if (current.op >= binary.code.size())
				return setErrorAndStop(ErrorCode::AVM_EEC_INVALID_JUMP);
		}

		/// @brief Jumps the operation pointer to the block's start.
		void jumpToBlockStart() {
			auto const start = current.start;
			current = {};
			current.start = start;
		}

		/// @brief Sets the AVM's current integer value.
		/// @param value Value to set.
		void setInt(ssize const value) {
			current.integer = value;
		}
		
		/// @brief Sets the AVM's current string value.
		/// @param value Value to set.
		void setString(String const& value) {
			current.string = value;
		}

		/// @brief Returns the AVM's current integer value.
		/// @return Current value.
		ssize getInt() {
			return current.integer;
		}
		
		/// @brief Returns the AVM's current string value.
		/// @return Current value.
		String getString() {
			return current.string;
		}

		/// @brief Forces an early return from the current block, if in any.
		/// @return Whether it was inside a block.
		bool forceBlockExit() {
			if (stack.empty()) return false;
			retrieveState();
			return true;
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
			/// @brief Start-of-block pointer.
			usize		start	= 0;
			/// @brief Operation pointer.
			usize		op		= 0;
			/// @brief Whether inside a function.
			bool		inFunc	= false;
			/// @brief Current integer.
			ssize		integer	= 0;
			/// @brief Current string.
			String		string	= "";
		};

		/// @brief Function stack frame.
		struct FunctionFrame {
			usize		name;
			StringList	values;
		};

		/// @brief Program stack.
		List<Frame>			stack;
		/// @brief Function stack.
		List<FunctionFrame>	funStack;
		/// @brief Current execution state.
		Frame				current;

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

		String parseSub(String const& arg) {
			String out = arg[0] == SUB_CHAR ? arg.substring(1) : arg;
			StringList const argInfo = out.splitAtLast('@');
			//DEBUGLN("Finding value for argument '", arg, "'...");
			try {
				usize const name	= argInfo[0].size() > 1 ? toUInt64(argInfo[0], 10) : argInfo[0][0] - '0';
				usize const index	= argInfo[1].size() > 1 ? toUInt64(argInfo[1], 10) : argInfo[1][0] - '0';
				//DEBUGLN("ARGUMENT: Func: ", name, ", Index: [", index, "]");
				for (usize i = funStack.size()-1; i < funStack.size(); --i) {
					if (funStack[i].name == name && index < funStack[i].values.size())
						return funStack[i].values[index];
				}
			} catch (FailedActionException const& e) {
				//DEBUGLN("Uh oh...");
				setErrorAndStop(ErrorCode::AVM_EEC_ARGUMENT_PARSE_FAILURE);
			}
			setErrorAndStop(ErrorCode::AVM_EEC_MISSING_FUNCTION_ARGUMENT);
			//DEBUGLN("Value not found");
			return "";
		}

		String parseReps(String const& str) {
			String out = "";
			bool inSub = false;
			//DEBUGLN("Parsing string...");
			for (auto const& bit: str.split(SUB_CHAR)) {
				if (inSub)
					out += parseSub(bit);
				else out += bit;
				inSub = !inSub;
			}
			//DEBUGLN("String parsed!");
			return out;
		}

		Parameters getArguments(usize const start, usize const count) {
			//DEBUGLN("Processing argument set [Start: ", start, ", Count: ", count+1, "]...");
			auto args = binary.data.sliced(start, start + count);
			//DEBUGLN("Before: ['", args.join("', '"), "']");
			for (auto& arg: args)
				if (arg.size())
					switch (arg[0]) {
						case SUB_CHAR: arg = parseSub(arg); break;
						case REP_CHAR: arg = parseReps(arg.substring(1)); break;
					}
			return args;
		}

		void storeState(usize const op) {
			storeState();
			stack.back().op = op;
		}

		void storeState() {
			stack.pushBack(current);
			auto const op	= current.op;
			current			= Frame();
			current.op		= op;
		}

		void retrieveState() {
			if (stack.empty()) return;
			if (current.inFunc) funStack.popBack();
			current	= stack.popBack();
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
				opAdd(current.actors, parseReps(binary.data[line]));
			else opSay(current.actors, line ? parseReps(binary.data[line]) : "");
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
			opPerform(current.actors, action, getArguments(params, psize));
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
			opNamedCallMultiple(param, getArguments(value, vcount));
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
			auto const choice = current.integer;
			if (spm & 0b1000) storeState(current.op + range * JUMP_SIZE);
			if (range > 0) {
				if ((spm & 0b0111) == 2) {
					DEBUGLN("Jump ID: ", choice);
					DEBUGLN("Range: ", range);
					current.op += Math::clamp<ssize>(choice, 0, range-1) * JUMP_SIZE;
				}
				else current.op += rng.integer<usize>(0, range-1) * JUMP_SIZE;
			}
		}

		void opGetValue() {
			uint64 name;
			if (!operand64(name)) return;
			auto spm = sp();
			if (spm == 3) {
				uint64 start, size;
				if (!operands64(start, size)) return;
				if (start)
					opGetChoice(name, getArguments(start, size));
				else opGetChoice(name, Parameters());
				return;
			}
			if (spm == 2) return opGetString(name);
			opGetInt(name);
		}

		void opInvoke() {
			uint64 name;
			if (!operand64(name)) return;
			auto spm = sp();
			funStack.pushBack({name});
			//DEBUGLN("Calling function [", name, "]...");
			if (spm) {
				uint64 args, count;
				if (!operands64(args, count)) return;
				funStack.back().values = getArguments(args, count);
				//DEBUGLN("Args: ['", funStack.back().values.join("', '"), "']");
				//DEBUGLN("Stack: [", funStack.size(), "]");
			}
			jumpTo(name);
			current.inFunc = true;
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
