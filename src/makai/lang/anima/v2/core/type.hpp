#ifndef MAKAILIB_ANIMA_V2_CORE_TYPE_H
#define MAKAILIB_ANIMA_V2_CORE_TYPE_H

#include "forward.hpp"
#include "entry.hpp"

namespace Makai::Anima::V2::Core {
	/// @brief Operator.
	enum class Operator: uint16 {
		AV2_UOP_NEGATE,
		AV2_UOP_INCREMENT,
		AV2_UOP_DECREMENT,
		AV2_UOP_INVERSE,
		AV2_UOP_LOGIC_NOT,
		AV2_UOP_BIT_NOT,
		AV2_UOP_SIN,
		AV2_UOP_COS,
		AV2_UOP_TAN,
		AV2_UOP_ASIN,
		AV2_UOP_ACOS,
		AV2_UOP_ATAN,
		AV2_UOP_SINH,
		AV2_UOP_COSH,
		AV2_UOP_TANH,
		AV2_UOP_LOG2,
		AV2_UOP_LOG10,
		AV2_UOP_LN,
		AV2_UOP_SQRT,
		AV2_UOP_LENGTH,
		AV2_BOP_START = 0x1000,
		AV2_BOP_ADD = AV2_BOP_START,
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
		AV2_BOP_LOGX,
		AV2_BOP_ATAN2,
		AV2_BOP_POW,
	};

	enum class BasicType {
		AV2_BT_NOT_A_BASIC_TYPE = -1,
		AV2_BT_VOID,
		AV2_BT_ANY,
		AV2_BT_NULL,
		AV2_BT_BOOL,
		AV2_BT_INT,
		AV2_BT_UINT,
		AV2_BT_REAL,
		AV2_BT_STRING,
		AV2_BT_BYTES,
		AV2_BT_VECTOR,
	};

	struct Definition: Entry {
		struct Flags {
			constexpr static uint64 const AV2_DF_BASIC			= 1 << 0;
			constexpr static uint64 const AV2_DF_NULLABLE		= 1 << 1;
			constexpr static uint64 const AV2_DF_EMPTY			= 1 << 2;
			constexpr static uint64 const AV2_DF_ARRAY			= 1 << 3;
			constexpr static uint64 const AV2_DF_VALUE			= 1 << 4;
			constexpr static uint64 const AV2_DF_STRUCTURE		= 1 << 5;
			constexpr static uint64 const AV2_DF_DYNAMIC		= 1 << 6;
			constexpr static uint64 const AV2_DF_CLONABLE		= 1 << 7;
			constexpr static uint64 const AV2_DF_ART_EQUIVALENT	= 1 << 8;
			constexpr static uint64 const AV2_DF_NO_RESULT		= 1 << 9;
			constexpr static uint64 const AV2_DF_POINTER		= 1 << 10;
			constexpr static uint64 const AV2_DF_FINAL			= 1 << 10;
		};

		bool canBecome(Instance<Definition> const& type) const {
			if (type == base) return true;
			Instance<Definition> current = base;
			while ((current = current->base))
				if (current == type) return true;
			return false;
		}

		uint64						flags		= 0;
		Nullable<BasicType>			basic;
		Instance<Definition>		base		= nullptr;
		uint64						byteSize	= 0;
		uint64						alignment	= 1;
		List<Instance<Definition>>	fields;

		Functor<void(ptr<void>)>							construct;
		Functor<void(ptr<void>, ptr<void const>)>			copy;
		Functor<void(ptr<void>)>							destruct;
		Functor<int64(ptr<void const>, ptr<void const>)>	compare;
	};
}

#endif
