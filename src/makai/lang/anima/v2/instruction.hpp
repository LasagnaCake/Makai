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
		AV2_OP_LESS_THAN,
		AV2_OP_GREATER_THAN,
		AV2_OP_EQUALS,
		AV2_OP_THREEWAY
	};
	
	/// @brief Instruction.
	struct [[gnu::aligned(8)]] Instruction {
		/// @brief Value declaration.
		struct [[gnu::aligned(4)]] Declaration {
			uint16 type;
			uint16 modifiers;
		};
		
		/// @brief Value transfer.
		struct [[gnu::aligned(4)]] Transfer {
			DataLocation	from, to;
			uint16			id;
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
		
		/// @brief Comparison operator.
		struct [[gnu::aligned(4)]] Comparison {
			DataLocation	output;
			Comparator		type;
		};
		
		/// @brief Return result.
		struct [[gnu::aligned(4)]] Result {
			bool			ignore;
			DataLocation	location;
		};

		/// @brief Stack interaction.
		struct [[gnu::aligned(4)]] StackInteraction {
			DataLocation	location;
		};

		/// @brief Math operation.
		struct [[gnu::aligned(4)]] BinaryMath {
			enum class Operation: uint8 {
				AV2_IBM_OP_ADD,
				AV2_IBM_OP_SUB,
				AV2_IBM_OP_MUL,
				AV2_IBM_OP_DIV,
				AV2_IBM_OP_REM,
				AV2_IBM_OP_POW,
			};
			Operation op;
			DataLocation lhs, rhs, out;
		};
		
		/// @brief Instruction name.
		enum class Name: uint32 {
			/// @brief No-operation.
			/// @param type 0 = Wastes a cycle; 1 = does not waste a cycle.
			AV2_IN_NO_OP,
			/// @brief No-operation.
			/// @param type `Result` = Where the result is located.
			AV2_IN_HALT,
			/// @brief Declare a global variable.
			/// @param type `Declaration` = How to declare the variable.
			AV2_IN_GLOBAL,
			/// @brief Copies a value from one location to another.
			/// @param type `Transfer` = How to transfer the data.
			AV2_IN_COPY,
			/// @brief Invokes a function.
			/// @param type `Invocation` = How to invoke the function.
			AV2_IN_CALL,
			/// @brief Pushes a value to the top of the stack.	
			/// @param type `StackInteraction` = How to handle the value.	
			AV2_IN_STACK_PUSH,
			/// @brief Pops a value from the top of the stack into a given location.	
			/// @param type `StackInteraction` = How to handle the value.
			AV2_IN_STACK_POP,
			/// @brief Swaps the topmost two values of the stack.	
			/// @param type Discarded.	
			AV2_IN_STACK_SWAP,
			/// @brief Clears a given number of elements from the top of the stack.
			/// @param type Amount of items to clear.
			AV2_IN_STACK_CLEAR,
			/// @brief Clears the entire stack.
			/// @param type Discarded.
			AV2_IN_STACK_FLUSH,
			/// @brief Returns from a function.
			/// @param type `Result` = How should the result be handled.
			AV2_IN_RETURN,
			/// @brief Executes a mathematical operation involving a unary operator.
			/// @param type `MathOperation` = How to process the math operation.
			AV2_IN_MATH_BOP
		};
		
		/// @brief Instruction "Name" (opcode).
		Name	name;
		/// @brief Instruction "Type" (specification).
		uint32	type;
		
		/// @brief Parses an instruction from a value.
		constexpr static Instruction fromValue(uint64 const v) {
			return CTL::bitcast<Instruction>(v);
		}
	};
}

#endif
