#ifndef MAKAILIB_ANIMA_V2_CORE_TYPE_H
#define MAKAILIB_ANIMA_V2_CORE_TYPE_H

#include "forward.hpp"

namespace Makai::Anima::V2::Core {
	/// @brief Operator.
	enum class Operator: uint16 {
		AV2_UOP_NEGATE,
		AV2_UOP_INCREMENT,
		AV2_UOP_DECREMENT,
		AV2_UOP_INVERSE,
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
		AV2_BOP_ADD = 0x1000,
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

	struct Definition {
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
		};
		StringList					aliases;
		uint64						flags		= 0;
		Nullable<BasicType>			basic		= BasicType::AV2_BT_NOT_A_BASIC_TYPE;
		Instance<Definition>		base		= nullptr;
		uint64						byteSize	= 0;
		uint64						alignment	= 1;
		List<Instance<Definition>>	fields;

		struct Database {
			using Type = Instance<Definition>;
			using StorageType = List<Type>;

			List<Type> byAlias(String const& alias) {
				StorageType defs;
				for (auto& type: types) {
					if (type->aliases.find(alias) != -1)
						defs.pushBack(type);
				}
				return defs;
			}

			Type byID(uint64 const id) {
				if (id < types.size())
					return types[id];
				return nullptr;
			}

			StorageType types;
		};

		Functor<void(ref<byte const>)>							construct;
		Functor<void(ref<byte>)>								destruct;
		Nullable<Function<void(ref<byte>, ref<byte const>)>>	clone;
	};
}

#endif
