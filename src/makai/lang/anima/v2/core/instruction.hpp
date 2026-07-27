#ifndef MAKAILIB_ANIMA_V2_CORE_INSTRUCTION_H
#define MAKAILIB_ANIMA_V2_CORE_INSTRUCTION_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	/// @brief Value location.
	union ValueLocation {
		/// @brief Value source.
		enum class Source: uint8 {
			/// @brief Null value.
			AV2_VLS_NULL,
			/// @brief Boolean value.
			AV2_VLS_BOOL,
			/// @brief Integer value.
			AV2_VLS_INT,
			/// @brief Floating point value.
			AV2_VLS_REAL,
			/// @brief Text value.
			AV2_VLS_STRING,
			/// @brief Value in absolute position in the global stack.
			AV2_VLS_STACK,
			/// @brief Value in offset from the top of the global stack.
			AV2_VLS_STACK_OFFSET,
			/// @brief Global value.
			AV2_VLS_GLOBAL,
			/// @brief Implemetation-defined value.
			AV2_VLS_EXTERNAL,
			/// @brief Scope-local value.
			AV2_VLS_LOCAL,
		};
		/// @brief Description of a constant empty value.
		struct [[gnu::packed, gnu::aligned(1)]] ForVoidOrNull {
			Source	source:	4;
		} forVoidOrNull;
		/// @brief Description of a constant boolean value.
		struct [[gnu::packed, gnu::aligned(1)]] ForBoolean {
			Source	source:	4;
			uint8	flag:	1;
		} forBool;
		/// @brief Description of an constant integer value.
		struct [[gnu::packed, gnu::aligned(1)]] ForInteger {
			enum class Size: uint8 {
				AV2_VL_IS_8_BIT,
				AV2_VL_IS_16_BIT,
				AV2_VL_IS_32_BIT,
				AV2_VL_IS_64_BIT
			};
			Source	source:		4;
			uint8	isUnsigned:	1;
			Size	size:		2;
		} forInt;
		/// @brief Description of a constant floating point value.
		struct [[gnu::packed, gnu::aligned(1)]] ForReal {
			enum class Size: uint8 {
				AV2_VL_RS_32_BIT,
				AV2_VL_RS_64_BIT,
				AV2_VL_RS_128_BIT
			};
			Source	source:	4;
			Size	size:	2;
		} forReal;
		/// @brief Description of a constant string value.
		struct [[gnu::packed, gnu::aligned(1)]] ForString {
			Source	source:	4;
		} forString;
		/// @brief Description of a non-constant value.
		struct [[gnu::packed, gnu::aligned(1)]] ForObject {
			enum class Transfer: uint8 {
				AV2_VL_OT_COPY,
				AV2_VL_OT_REF,
				AV2_VL_OT_MOVE,
			};
			Source		source:		4;
			Transfer	transfer:	2;
		} forObject;
		/// @brief Generic descriptor of a value.
		struct [[gnu::packed, gnu::aligned(1)]] Description {
			Source	source:		4;
			uint8	modifiers:	4;
		} desc;
		/// @brief Location as integer.
		uint8 value;
		/// @brief Creates a value location for a given source.
		/// @param source Source to create location for.
		/// @return Location for source.
		constexpr static ValueLocation fromSource(Source const source) {
			return ValueLocation{
				.desc = {
					.source = source
				}
			};
		}
	};

	static_assert(sizeof(ValueLocation) == sizeof(uint8));

	/// @brief Address jump mode.
	enum class JumpMode: uint8 {
		AV2_JM_TABLE_INDEX,
		AV2_JM_RELATIVE,
		AV2_JM_ABSOLUTE,
	};

	/// @brief Execution context mode.
	enum class ContextMode: uint8 {
		/// @brief Strict context.
		AV2_CM_STRICT,
		/// @brief Loose context.
		AV2_CM_LOOSE,
	};

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
			uint8		immediate:	1;
		};

		/// @brief Value transfer.
		struct [[gnu::aligned(4)]] Transfer {
			ValueLocation	from, to;
		};

		/// @brief Function invocation.
		struct [[gnu::aligned(4)]] Invocation {
			uint8	dynamic:	1;
			uint8	external:	1;
			uint8	optional:	1;
			uint8	noResult:	1;
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
				AV2_ILT_IF_NULL_OR_VOID,
				AV2_ILT_IF_EXISTS,
			};
			using Mode = JumpMode;
			Type	type:	4;
			uint8	dyn:	1;
			Mode	mode:	2;
			uint8	invert:	1;
		};

		/// @brief Comparison operator.
		struct [[gnu::aligned(4)]] Comparison {
			Comparator	comp;
			uint8		sameType:	1;
			BasicType	assume:		7;
		};

		/// @brief Stack push.
		struct [[gnu::aligned(4)]] StackPush {
			ValueLocation	location;
		};

		/// @brief Operation.
		struct [[gnu::aligned(4)]] Operation {
			Operator		op;
			uint8			sameType:	1;
			BasicType		assume:		7;
			uint8			count		= 1;
			uint8			immediate:	1;
		};

		/// @brief Blitting.
		struct [[gnu::aligned(4)]] Blitting {
			enum class Type: uint8 {
				AV2_IBT_COPY,
				AV2_IBT_REFERENCE,
				AV2_IBT_MOVE
			};
			Type 	type:		7;
			uint8	fromGlobal:	1;
			uint16	offset;
		};

		struct [[gnu::aligned(4)]] Binding {
			uint32	src:	15;
			uint32	dst:	15;
			uint32	copy:	1;
		};

		struct [[gnu::aligned(4)]] Casting {
			uint8 dynamic:	1;
			uint8 noCopy:	1;
			uint8 unsafe:	1;
		};

		struct [[gnu::aligned(4)]] Field {
			uint8 dynamic: 1;
		};

		struct [[gnu::aligned(4)]] Waiting {
			uint8 dynamic:	1;
			uint8 once:		1;
		};

		/// @brief Randomness.
		struct [[gnu::aligned(4)]] Randomness {
			enum class Type: uint8 {
				AV2_IRT_INT,
				AV2_IRT_UINT,
				AV2_IRT_REAL,
			};

			Type	type:		2;
			uint8	secure:		1;
			uint8	bounded:	1;
			uint8	getSeed:	1;
			uint8	setSeed:	1;
		};

		struct [[gnu::aligned(4)]] Clear {
			ValueLocation	at;
			uint8			dyn: 1;
		};

		struct [[gnu::aligned(4)]] Create {
			uint8	dyn:			1;
			uint8	forArray:		1;
			uint8	dynSize:		1;
			uint8	andInit:		1;
		};

		struct [[gnu::aligned(4)]] Selection {
			uint16		count;
			JumpMode	mode:	2;
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
			/// @brief Extends the end of the stack with empty values.
			/// @param type Amount of values add to stack.
			/// @details `grow <count>`
			AV2_IN_STACK_GROW,
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
			/// @param type `Waiting` = How to handle the waiting.
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
			/// @brief Adds an extra count of entries at the end of the local stack.
			/// @param type Amount of entries to add.
			/// @details `decl`
			AV2_IN_SCOPE_DECLARE,
			/// @brief Binds all of the previous scope's values to this one.
			/// @param type Discarded.
			/// @details `keep`
			AV2_IN_SCOPE_KEEP,
			/// @brief Gets a reference of a given field from an object or array.
			/// @param type `Field` = how to access the field.
			/// @details `get [<id>]`
			AV2_IN_FIELD_GET,
			/// @brief Sets a given field of an object or array with a given value.
			/// @param type `Field` = how to access the field.
			/// @details `set [<id>]`
			AV2_IN_FIELD_SET,
			/// @brief Gets the size of a value.
			/// @param type 0 = element count, 1 = in bytes.
			/// @details `size`
			AV2_IN_SIZEOF,
			/// @brief Gets the type of a value.
			/// @param type Discarded.
			/// @details `type`
			AV2_IN_TYPEOF,
			/// @brief Jumps to one of the given targets, depending on the topmost value in the stack.
			/// @param type Amount of jump targets.
			/// @details `select <jump-id> ...`
			AV2_IN_SELECT,
			/// @brief Clears a given location.
			/// @param type `Clear` = how to clear the location.
			/// @details `clear [<loc-id>]`
			AV2_IN_CLEAR,
			/// @brief Creates an empty value with the given type and pushes it to the stack.
			/// @param type `Create` = how to create the value.
			/// @details `new [<type>]`
			AV2_IN_CREATE,
			/// @brief Initializes the current value on the stack with the current scope's local values.
			/// @param type Discarded.
			/// @details `init`
			AV2_IN_INITIALIZE,
			/// @brief Sends a debug breakpoint.
			/// @param type Discarded.
			/// @details `break`
			AV2_IN_BREAKPOINT,
		};

		/// @brief Instruction "Name" (opcode).
		Name	name;
		/// @brief Instruction "Type" (specification).
		uint32	type;

		constexpr static String asString(Name const& name) {
			switch (name) {
				case Name::AV2_IN_NO_OP:			return "nop";
				case Name::AV2_IN_HALT:				return "halt";
				case Name::AV2_IN_MODE:				return "mode";
				case Name::AV2_IN_COPY:				return "copy";
				case Name::AV2_IN_COMPARE:			return "compare";
				case Name::AV2_IN_CALL:				return "call";
				case Name::AV2_IN_JUMP:				return "jump";
				case Name::AV2_IN_STACK_PUSH:		return "push";
				case Name::AV2_IN_STACK_POP:		return "pop";
				case Name::AV2_IN_STACK_CLEAR:		return "clear";
				case Name::AV2_IN_STACK_FLUSH:		return "flush";
				case Name::AV2_IN_STACK_SWAP:		return "swap";
				case Name::AV2_IN_STACK_GROW:		return "grow";
				case Name::AV2_IN_STACK_BLIT:		return "blit";
				case Name::AV2_IN_RETURN:			return "return";
				case Name::AV2_IN_OP:				return "op";
				case Name::AV2_IN_YIELD:			return "yield";
				case Name::AV2_IN_CAST:				return "cast";
				case Name::AV2_IN_RANDOM:			return "rng";
				case Name::AV2_IN_SCOPE_ENTER:		return "enter";
				case Name::AV2_IN_SCOPE_EXIT:		return "exit";
				case Name::AV2_IN_SCOPE_BIND:		return "bind";
				case Name::AV2_IN_SCOPE_BRING:		return "bring";
				case Name::AV2_IN_SCOPE_DECLARE:	return "decl";
				case Name::AV2_IN_SCOPE_KEEP:		return "keep";
				case Name::AV2_IN_FIELD_GET:		return "get";
				case Name::AV2_IN_FIELD_SET:		return "set";
				case Name::AV2_IN_SIZEOF:			return "sizeof";
				case Name::AV2_IN_TYPEOF:			return "typeof";
				case Name::AV2_IN_SELECT:			return "pick";
				case Name::AV2_IN_CLEAR:			return "drop";
				case Name::AV2_IN_CREATE:			return "new";
				case Name::AV2_IN_INITIALIZE:		return "init";
				case Name::AV2_IN_BREAKPOINT:		return "break";
				case Name::AV2_IN_TIME:				return "time";
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
