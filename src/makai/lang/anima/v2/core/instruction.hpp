#ifndef MAKAILIB_ANIMA_V2_CORE_INSTRUCTION_H
#define MAKAILIB_ANIMA_V2_CORE_INSTRUCTION_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	/// @brief Data location.
	enum class DataLocation: uint8 {
		/// @brief Void value.
		AV2_DL_VOID,
		/// @brief Null value.
		AV2_DL_NULL,
		/// @brief Boolean value.
		AV2_DL_BOOL,
		/// @brief Unsigned integer value.
		AV2_DL_UINT,
		/// @brief Signed integer value.
		AV2_DL_INT,
		/// @brief Floating point value.
		AV2_DL_REAL,
		/// @brief Text value.
		AV2_DL_STRING,
		/// @brief Value in absolute position in the global stack.
		AV2_DL_STACK,
		/// @brief Value in offset from the top of the global stack.
		AV2_DL_STACK_OFFSET,
		/// @brief Global value.
		AV2_DL_GLOBAL,
		/// @brief Implemetation-defined value.
		AV2_DL_EXTERNAL,
		/// @brief Scope-local value.
		AV2_DL_LOCAL,
		/// @brief Location modifier: By reference.
		AV2_DLM_BY_REF	= 0b10000000,
		/// @brief Location modifier: By move.
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

	/// @brief Returns the data location without modifiers.
	constexpr DataLocation asPlace(DataLocation const loc) {
		return (loc & ~(DataLocation::AV2_DLM_BY_REF | DataLocation::AV2_DLM_MOVE));
	}

	/// @brief Returns the modifiers for a data location.
	constexpr DataLocation asModifiers(DataLocation const loc) {
		return (loc & (DataLocation::AV2_DLM_BY_REF | DataLocation::AV2_DLM_MOVE));
	}

	/// @brief Comparison operator.
	enum class Comparator: uint8 {
		AV2_OP_THREEWAY,
		AV2_OP_NOT_EQUALS,
		AV2_OP_EQUALS,
		AV2_OP_LESS_THAN,
		AV2_OP_GREATER_THAN,
		AV2_OP_LESS_EQUALS,
		AV2_OP_GREATER_EQUALS,
	};

	/// @brief Instruction.
	struct [[gnu::aligned(8)]] Instruction {
		/// @brief Stop mode.
		struct [[gnu::aligned(4)]] Stop {
			enum class Mode: uint8 {
				AV2_ISM_NORMAL,
				AV2_ISM_ERROR
			};
			Mode	mode;
		};

		/// @brief Context mode.
		struct [[gnu::aligned(4)]] Context {
			ContextMode	mode;
			bool		immediate:	1;
		};

		/// @brief Value transfer.
		struct [[gnu::aligned(4)]] Transfer {
			DataLocation	from, to;
		};

		/// @brief Function invocation.
		struct [[gnu::aligned(4)]] Invocation {
			bool	dynamic:	1;
			bool	external:	1;
		};

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
				AV2_ILT_IF_UNDEFINED,
				AV2_ILT_IF_NULL_OR_UNDEFINED,
			};
			Type	type;
			bool	dyn: 1;
		};

		/// @brief Comparison operator.
		struct [[gnu::aligned(4)]] Comparison {
			Comparator	comp;
		};

		/// @brief Stack push.
		struct [[gnu::aligned(4)]] StackPush {
			DataLocation	location;
		};

		/// @brief Operation.
		struct [[gnu::aligned(4)]] Operation {
			Operator op;
		};

		/// @brief Blitting.
		struct [[gnu::aligned(4)]] Blitting {
			enum class Type: uint8 {
				AV2_IBT_COPY,
				AV2_IBT_REFERENCE,
				AV2_IBT_MOVE
			};
			Type 	type:		7;
			bool	fromGlobal:	1;
			uint16	offset;
		};

		struct [[gnu::aligned(4)]] Binding {
			uint16	src;
			uint16	dst;
		};

		struct [[gnu::aligned(4)]] Casting {
			bool dynamic:	1;
			bool unsafe:	1;
		};

		struct [[gnu::aligned(4)]] Field {
			bool dynamic: 1;
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
				AV2_IRF_GET_SEED	= 1 << 3,
			};

			Type	type:		2;
			bool	secure:		1;
			bool	bounded:	1;
			bool	getSeed:	1;
			bool	setSeed:	1;
		};

		struct [[gnu::aligned(4)]] Clear {
			DataLocation	at;
			bool			dyn: 1;
		};

		/// @brief Instruction name.
		enum class Name: uint32 {
			/// @brief No-operation.
			/// @param type 0 = Wastes a cycle; 1 = does not waste a cycle.
			/// @details `nop`
			AV2_IN_NO_OP,
			/// @brief Halts execution.
			/// @param type `Stop` = What kind of stop to do.
			/// @details `halt`
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
			/// @param type Comparator to use.
			/// @details `compare`
			AV2_IN_COMPARE,
			/// @brief Invokes a function.
			/// @param type `Invocation` = How to invoke the function.
			/// @details `call [<func-id>]`
			AV2_IN_CALL,
			/// @brief Executes a jump.
			/// @param type `Leap` = How to jump.
			/// @details `jump [<to-id>]`
			AV2_IN_JUMP,
			/// @brief Pushes a value to the top of the global stack.
			/// @param type `StackPush` = How to handle the value.
			/// @details `push [<loc-id>]`
			AV2_IN_STACK_PUSH,
			/// @brief Pops a value from the top of the global stack.
			/// @details `pop`
			AV2_IN_STACK_POP,
			/// @brief Swaps the topmost two values of the global stack.
			/// @param type Discarded.
			/// @details `swap`
			AV2_IN_STACK_SWAP,
			/// @brief Clears a given number of elements from the top of the global stack.
			/// @param type Discarded.
			/// @details `clear <count>`
			AV2_IN_STACK_CLEAR,
			/// @brief Clears the entire global stack.
			/// @param type Discarded.
			/// @details `flush`
			AV2_IN_STACK_FLUSH,
			/// @brief Copies a set of values from one stack to another.
			/// @param type `Blitting` = how to blit the values.
			/// @note Values are transfered "front"-to-"back", offset starts at the end of the stack for global stack.
			/// @details `blit <count>`
			AV2_IN_STACK_BLIT,
			/// @brief Returns from a function.
			/// @param type Discarded.
			/// @details `return`
			AV2_IN_RETURN,
			/// @brief Executes an operation involving a, operator.
			/// @param type `Operation` = How to operate.
			/// @details `op`
			AV2_IN_OP,
			/// @brief Returns execution to the engine.
			/// @param type Discarded.
			/// @details `yield`
			AV2_IN_YIELD,
			/// @brief Casts a given value to another type.
			/// @param type `Casting` = How to do the cast.
			/// @details `cast [<type-id>]`
			AV2_IN_CAST,
			/// @brief Generates a random number.
			/// @param type `Randomness` = How to generate the number.
			/// @details `rng`
			AV2_IN_RANDOM,
			/// @brief Declares a new scope.
			/// @param type Size of scope-local stack.
			/// @details `enter`
			AV2_IN_SCOPE_ENTER,
			/// @brief Pops the current scope off the stack.
			/// @param type Discarded.
			/// @details `exit`
			AV2_IN_SCOPE_EXIT,
			/// @brief Binds a range of values in the global stack to a range of places in the local stack.
			/// @param type `Binding` = how to bind the values.
			/// @details `bind <count>`
			AV2_IN_SCOPE_BIND,
			/// @brief Binds a range of values from a previous scope to a range of places in the current scope.
			/// @param type `Binding` = how to bind the values.
			/// @note Values are bound "front"-to-"back", source offset starts at the end of the stack.
			/// @details `bring <scope> <count>`
			AV2_IN_SCOPE_BRING,
			/// @brief Gets a reference of a given field from an object or array.
			/// @param type `Field` = how to access the field.
			/// @details `get <id>`
			AV2_IN_FIELD_GET,
			/// @brief Sets a given field of an object or array with a given value.
			/// @param type `Field` = how to access the field.
			/// @details `set <id>`
			AV2_IN_FIELD_SET,
			/// @brief Gets the size of a value.
			/// @param type 0 = element count, 1 = in bytes.
			/// @details `size`
			AV2_IN_SIZEOF,
			/// @brief Gets the type of a value.
			/// @param type Discarded.
			/// @details `type`
			AV2_IN_TYPEOF,
			/// @brief Clears a given location.
			/// @param type `Clear`= how to clear the value.
			/// @details `clear [<loc-id>]`
			AV2_IN_CLEAR,
			/// @brief Jumps to one of the given targets, depending on the topmost value in the stack.
			/// @param type Amount of jump targets.
			/// @details `select <loc-id> ...`
			AV2_IN_SELECT,
		};

		/// @brief Instruction "Name" (opcode).
		Name	name;
		/// @brief Instruction "Type" (specification).
		uint32	type;

		constexpr static String asString(Name const& name) {
			switch (name) {
				case Name::AV2_IN_NO_OP:		return "nop";
				case Name::AV2_IN_HALT:			return "halt";
				case Name::AV2_IN_MODE:			return "mode";
				case Name::AV2_IN_COPY:			return "copy";
				case Name::AV2_IN_COMPARE:		return "compare";
				case Name::AV2_IN_CALL:			return "call";
				case Name::AV2_IN_JUMP:			return "jump";
				case Name::AV2_IN_STACK_PUSH:	return "push";
				case Name::AV2_IN_STACK_POP:	return "pop";
				case Name::AV2_IN_STACK_CLEAR:	return "clear";
				case Name::AV2_IN_STACK_FLUSH:	return "flush";
				case Name::AV2_IN_STACK_SWAP:	return "swap";
				case Name::AV2_IN_STACK_BLIT:	return "blit";
				case Name::AV2_IN_RETURN:		return "return";
				case Name::AV2_IN_OP:			return "op";
				case Name::AV2_IN_YIELD:		return "yield";
				case Name::AV2_IN_CAST:			return "cast";
				case Name::AV2_IN_RANDOM:		return "rng";
				case Name::AV2_IN_SCOPE_ENTER:	return "enter";
				case Name::AV2_IN_SCOPE_EXIT:	return "exit";
				case Name::AV2_IN_SCOPE_BIND:	return "bind";
				case Name::AV2_IN_SCOPE_BRING:	return "bring";
				case Name::AV2_IN_FIELD_GET:	return "get";
				case Name::AV2_IN_FIELD_SET:	return "set";
				case Name::AV2_IN_SIZEOF:		return "sizeof";
				case Name::AV2_IN_TYPEOF:		return "typeof";
				case Name::AV2_IN_CLEAR:		return "drop";
				case Name::AV2_IN_SELECT:		return "pick";
			}
			return "???";
		}

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

	using Bytecode = List<Core::Instruction>;
}

#endif
