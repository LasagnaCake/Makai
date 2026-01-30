#ifndef MAKAILIB_ANIMA_V2_INSTRUCTION_H
#define MAKAILIB_ANIMA_V2_INSTRUCTION_H

#include "../../../compat/ctl.hpp"

namespace Makai::Anima::V2 {
	/// @brief Register count.
	constexpr usize REGISTER_COUNT = 32;

	/// @brief Data location.
	enum class DataLocation: uint8 {
		/// @brief Internal value.
		AV2_DL_INTERNAL,
		/// @brief Constant data.		
		AV2_DL_CONST,
		/// @brief Absolute position in the stack.
		AV2_DL_STACK,
		/// @brief Offset from the top of the stack.
		AV2_DL_STACK_OFFSET,
		/// @brief Heap.
		AV2_DL_HEAP,
		/// @brief Global variable.
		AV2_DL_GLOBAL,
		/// @brief C++ value.	
		AV2_DL_EXTERNAL,
		/// @brief Temporary register.	
		AV2_DL_TEMPORARY,
		/// @brief Register value.
		AV2_DL_REGISTER,
		/// @brief Location modifier: By reference.
		AV2_DLM_BY_REF	= 0b10000000,
		AV2_DLM_MOVE	= 0b01000000,
	};

	constexpr DataLocation operator|(DataLocation const& a, DataLocation const& b) {
		return Cast::as<DataLocation>(enumcast(a) | enumcast(b));
	}

	constexpr DataLocation operator&(DataLocation const& a, DataLocation const& b) {
		return Cast::as<DataLocation>(enumcast(a) & enumcast(b));
	}

	constexpr DataLocation operator~(DataLocation const& a) {
		return Cast::as<DataLocation>(enumcast(a));
	}

	/// @brief Execution context mode.
	enum class ContextMode: uint8 {
		/// @brief Strict context.
		AV2_CM_STRICT,
		/// @brief Loose context.
		AV2_CM_LOOSE,
	};

	/// @brief Returns the register for the given ID.
	/// @param id ID to get register for.
	/// @return Register for ID.
	constexpr DataLocation asRegister(usize const id) {
		return Cast::as<DataLocation>(enumcast(DataLocation::AV2_DL_REGISTER) + id);
	}

	/// @brief Data modifier.
	enum class DataModifier: uint16 {
		AV2_DM_REFERENCE	= 1 << 0,
		AV2_DM_TEMPORARY	= 1 << 1,
		AV2_DM_POINTER		= 1 << 2,
		AV2_DM_IN			= 1 << 3,
		AV2_DM_OUT			= 1 << 4,
		AV2_DM_CONST		= 1 << 5,
		AV2_DM_COMPILEABLE	= 1 << 6,
		AV2_DM_COMPILED		= 1 << 7
	};

	/// @brief Operator overloadings.
	constexpr DataModifier operator|(DataModifier const& a, DataModifier const& b)	{return Cast::as<DataModifier>(enumcast(a) | enumcast(b));	}
	constexpr DataModifier operator&(DataModifier const& a, DataModifier const& b)	{return Cast::as<DataModifier>(enumcast(a) & enumcast(b));	}
	constexpr DataModifier operator~(DataModifier const& a)							{return Cast::as<DataModifier>(~enumcast(a));				}
	
	/// @brief Unary operator.
	enum class UnaryOperator: uint8 {
		AV2_UOP_NEGATE,
		AV2_UOP_LOGIC_NOT,
		AV2_UOP_BIT_NOT,
		AV2_UOP_NEW,
		AV2_UOP_DELETE,
		AV2_UOP_COPY,
		AV2_UOP_MOVE,
	};
	
	/// @brief Binary operator.
	enum class BinaryOperator: uint8 {
		AV2_BOP_ADD,
		AV2_BOP_SUB,
		AV2_BOP_MUL,
		AV2_BOP_DIV,
		AV2_BOP_REM,
		AV2_BOP_LOGIC_AND,
		AV2_BOP_LOGIC_OR,
		AV2_BOP_LOGIC_XOR,
		AV2_BOP_BIT_AND,
		AV2_BOP_BIT_OR,
		AV2_BOP_BIT_XOR,
		AV2_BOP_ARRAY_GET,
		AV2_BOP_MEMBER_GET,
		AV2_BOP_NULL_DECAY,
		AV2_BOP_INDEX_ACCESS,
		AV2_BOP_MEMBER_ACCESS
	};
	
	/// @brief Comparison operator.
	enum class Comparator: uint8 {
		AV2_OP_EQUALS,
		AV2_OP_NOT_EQUALS,
		AV2_OP_LESS_THAN,
		AV2_OP_GREATER_THAN,
		AV2_OP_LESS_EQUALS,
		AV2_OP_GREATER_EQUALS,
		AV2_OP_THREEWAY,
		AV2_OP_TYPE_COMPARE,
	};
	enum class StringOperation: uint8 {
		AV2_OP_JOIN,
		AV2_OP_SPLIT,
		AV2_OP_REPLACE,
		AV2_OP_REMOVE,
		AV2_OP_SUBSTRING,
		AV2_OP_MATCH,
	};
	
	/// @brief Instruction.
	struct [[gnu::aligned(8)]] Instruction {
		/// @brief Stop mode.
		struct [[gnu::aligned(4)]] Stop {
			enum class Mode: uint8 {
				AV2_ISM_NORMAL,
				AV2_ISM_ERROR
			};
			Mode			mode;
			DataLocation	source;
		};

		/// @brief Context mode.
		struct [[gnu::aligned(4)]] Context {
			ContextMode		mode;
			bool			immediate:	1;
		};
		
		/// @brief Value transfer.
		struct [[gnu::aligned(4)]] Transfer {
			DataLocation	from, to;
		};
		
		/// @brief Function invocation.
		struct [[gnu::aligned(4)]] Invocation {
			DataLocation	location;
			uint8			argc;
			
			/// @brief Parameter declaration.
			struct [[gnu::aligned(8)]] Parameter {
				DataLocation	location;
				uint8			argument;
				uint32			id;
			};
		};
		
		/// @brief Function invocation.
		struct [[gnu::aligned(4)]] ReferenceCall: Invocation {};

		/// @brief Jump leap.
		struct [[gnu::aligned(4)]] Leap {
			enum class Type: uint8 {
				AV2_ILT_UNCONDITIONAL,
				AV2_ILT_IF_TRUTHY,
				AV2_ILT_IF_FALSY,
				AV2_ILT_IF_ZERO,
				AV2_ILT_IF_NOT_ZERO,
				AV2_ILT_IF_NEGATIVE,
				AV2_ILT_IF_POSITIVE,
				AV2_ILT_IF_NULL,
				AV2_ILT_IF_NAN,
				AV2_ILT_IF_UNDEFINED,
				AV2_ILT_IF_NULL_OR_UNDEFINED,
			};
			Type			type:		7;
			bool			isDynamic:	1;
			DataLocation	condition;
		};
		
		/// @brief Comparison operator.
		struct [[gnu::aligned(4)]] Comparison {
			DataLocation	lhs, rhs, out;
			Comparator		comp;
		};
		
		/// @brief Return result.
		struct [[gnu::aligned(4)]] Result {
			DataLocation	location;
			bool			ignore:		1;
		};

		/// @brief Stack push.
		struct [[gnu::aligned(4)]] StackPush {
			DataLocation	location;
		};

		/// @brief Stack push.
		struct [[gnu::aligned(4)]] StackPop {
			DataLocation	location;
			bool			discard:	1;
		};

		/// @brief Binary math operation.
		struct [[gnu::aligned(4)]] BinaryMath {
			enum class Operation: uint8 {
				AV2_IBM_OP_ADD,
				AV2_IBM_OP_SUB,
				AV2_IBM_OP_MUL,
				AV2_IBM_OP_DIV,
				AV2_IBM_OP_REM,
				AV2_IBM_OP_POW,
				AV2_IBM_OP_ATAN2,
				AV2_IBM_OP_LOG,
				AV2_IBM_OP_ELSE,
			};
			Operation op;
			DataLocation lhs, rhs, out;
		};

		/// @brief Unary math operation.
		struct [[gnu::aligned(4)]] UnaryMath {
			enum class Operation: uint8 {
				AV2_IUM_OP_NEGATE,
				AV2_IUM_OP_INCREMENT,
				AV2_IUM_OP_DECREMENT,
				AV2_IUM_OP_INVERSE,
				AV2_IUM_OP_SIN,
				AV2_IUM_OP_COS,
				AV2_IUM_OP_TAN,
				AV2_IUM_OP_ASIN,
				AV2_IUM_OP_ACOS,
				AV2_IUM_OP_ATAN,
				AV2_IUM_OP_SINH,
				AV2_IUM_OP_COSH,
				AV2_IUM_OP_TANH,
				AV2_IUM_OP_LOG2,
				AV2_IUM_OP_LOG10,
				AV2_IUM_OP_LN,
				AV2_IUM_OP_SQRT,
			};
			Operation op;
			DataLocation v, out;
		};

		/// @brief Wait request.
		struct [[gnu::aligned(4)]] WaitRequest {
			enum class Wait: uint8 {
				AV2_IWRW_TRUTHY,
				AV2_IWRW_FALSY
			};

			DataLocation	val;
			Wait			wait;
		};

		/// @brief Field get request.
		struct [[gnu::aligned(4)]] GetRequest {
			DataLocation	from, to, field;
		};
		
		/// @brief Field set request.
		struct [[gnu::aligned(4)]] SetRequest {
			DataLocation	from, to, field;
		};
		
		/// @brief Cast operation.
		struct [[gnu::aligned(4)]] Casting {
			DataLocation		src, dst;
			Data::Value::Kind	type;
		};
		
		/// @brief String manipulation.
		struct [[gnu::aligned(4)]] StringManipulation {
			DataLocation	src, lhs, rhs, out;
		};
		
		/// @brief Object.
		struct [[gnu::aligned(4)]] Object {
			DataLocation	desc, out;
		};
		
		/// @brief Randomness.
		struct [[gnu::aligned(4)]] Randomness {
			enum class Type: uint8 {
				AV2_IRT_INT,
				AV2_IRT_UINT,
				AV2_IRT_REAL,
			};
			
			enum class Flags: uint8 {
				AV2_IRF_NONE		= 0,
				AV2_IRF_SECURE		= 1 << 0,
				AV2_IRF_BOUNDED		= 1 << 1,
				AV2_IRF_SET_SEED	= 1 << 2,
			};

			Type			type;
			Flags			flags = Flags::AV2_IRF_NONE;
			DataLocation	num;

			struct [[gnu::aligned(8)]] Number {
				DataLocation	lo, hi;
			};
		};
		
		/// @brief Instruction name.
		enum class Name: uint32 {
			/// @brief No-operation.
			/// @param type 0 = Wastes a cycle; 1 = does not waste a cycle.
			/// @details `nop`
			AV2_IN_NO_OP,
			/// @brief Halts execution.
			/// @param type `Stop` = What kind of stop to do.
			/// @details `halt [<source-id>]`
			AV2_IN_HALT,
			/// @brief Switches to a given execution context mode.
			/// @param type `Context` = What kind of context to switch to.
			/// @details `mode`
			AV2_IN_MODE,
			/// @brief Copies a value from one location to another.
			/// @param type `Transfer` = How to transfer the data.
			/// @details `copy [<from-id>] [<to-id>]`
			AV2_IN_COPY,
			/// @brief Performs a three-way comparison on two values.
			/// @param type `Comparison` = How to compare.
			/// @details `compare [<lhs-id>] [<rhs-id>] [<out-id>]`
			AV2_IN_COMPARE,
			/// @brief Invokes a function.
			/// @param type `Invocation` = How to invoke the function.
			/// @details `call [<func-id>] [<args> ...]`
			AV2_IN_CALL,
			/// @brief Executes a jump.
			/// @param type `Leap` = How to jump.
			/// @details `jump ([<dynamic-to-src-id>] [<cond-id>] | [<cond-id>] <to-id>)`
			AV2_IN_JUMP,
			/// @brief Pushes a value to the top of the stack.	
			/// @param type `StackPush` = How to handle the value.
			/// @details `push [<loc-id>]`
			AV2_IN_STACK_PUSH,
			/// @brief Pops a value from the top of the stack into a given location.
			/// @param type `StackPop` = How to handle the value.
			/// @details `pop [<loc-id>]`
			AV2_IN_STACK_POP,
			/// @brief Swaps the topmost two values of the stack.	
			/// @param type Discarded.
			/// @details `swap`
			AV2_IN_STACK_SWAP,
			/// @brief Clears a given number of elements from the top of the stack.
			/// @param type Amount of items to clear.
			/// @details `clear`
			AV2_IN_STACK_CLEAR,
			/// @brief Clears the entire stack.
			/// @param type Discarded.
			/// @details `flush`
			AV2_IN_STACK_FLUSH,
			/// @brief Returns from a function.
			/// @param type `Result` = How should the result be handled.
			/// @details `return [<result-id>]`
			AV2_IN_RETURN,
			/// @brief Executes a mathematical operation involving a binary operator.
			/// @param type `BinaryMath` = How to process the math operation.
			/// @details `bop [<lhs-id>] [rhs-id] [out-id]`
			AV2_IN_MATH_BOP,
			/// @brief Executes a mathematical operation involving a unary operator.
			/// @param type `UnaryMath` = How to process the math operation.
			/// @details `uop [<val-id>] [out-id]`
			AV2_IN_MATH_UOP,
			/// @brief Returns execution to the engine.
			/// @param type Discarded.
			/// @details `yield`
			AV2_IN_YIELD,
			/// @brief Awaits a given value to be in a certain state.
			/// @param type `WaitRequest` = What to expect from the value.
			/// @details `await [<loc-id>]`
			AV2_IN_AWAIT,
			/// @brief Gets the value of a field from an object.
			/// @param type `GetRequest` = How to get the value.
			/// @details `get [<path-id>] [<from-id>] [<to-id>]`
			AV2_IN_GET,
			/// @brief Sets the value of a field in an object.
			/// @param type `SetRequest` = How to set the value.
			/// @details `set [<path-id>] [<from-id>] [<to-id>]`
			AV2_IN_SET,
			/// @brief Casts a given value to another type.
			/// @param type `Casting` = How to cast the value.
			/// @details `cast [<src-id>] [<dst-id>]`
			AV2_IN_CAST,
			/// @brief Performs a string manipulation operation on a value.
			/// @param type `StringManipulation` = How to manipulate the string.
			/// @details `str <op-id> [<src-id>] [<lhs-id>] [<rhs-id>] [<out-id>]`
			AV2_IN_STR_OP,
			/// @brief Creates an object based on an object descriptor.
			/// @param type `Object` = How to create the object.
			/// @details `new [<desc-id>] [<loc-id>]`
			AV2_IN_NEW_OBJ,
			/// @brief Dinamically calls a function.
			/// @param type `Invocation` = How to call the function.
			/// @details `rcall [<src-func-id>] [<args> ...]`
			AV2_IN_DYN_CALL,
			/// @brief Generates a random number.
			/// @param type `Randomness` = How to generate the number.
			/// @details `rng [<num:Number> [<lo-id>] [<hi-id>]] [<num-id>]`
			AV2_IN_RANDOM,
		};
		
		/// @brief Instruction "Name" (opcode).
		Name	name;
		/// @brief Instruction "Type" (specification).
		uint32	type;
		
		/// @brief Parses an instruction from a value.
		constexpr static Instruction fromValue(uint64 const v) {
			return CTL::bitcast<Instruction>(v);
		}
		template <class T>
		constexpr void setType(T const& v) const
		requires (sizeof(T) == 4) {
			type = Cast::bit<decltype(type)>(v);
		}

		template <class T>
		constexpr T getTypeAs() const
		requires (sizeof(T) == 4) {
			return Cast::bit<T>(type);
		}

		template <class T>
		constexpr T as() const
		requires (sizeof(T) == 8) {
			return Cast::bit<T>(*this);
		}
	};
}

#endif
