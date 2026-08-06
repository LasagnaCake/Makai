#ifndef MAKAILIB_ANIMA_V2_CORE_TYPE_H
#define MAKAILIB_ANIMA_V2_CORE_TYPE_H

#include "forward.hpp"
#include "entry.hpp"

namespace Makai::Anima::V2::Core {
	/// @brief Operator.
	enum class Operator: uint8 {
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
		AV2_UOP_POP,
		AV2_BOP_START = 1 << 6,
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
		AV2_BOP_PUSH,
		AV2_TOP_START = 2 << 6,
		AV2_QOP_START = 3 << 6,
	};

	enum class BasicType: int8 {
		AV2_BT_NOT_A_BASIC_TYPE = -1,
		AV2_BT_VOID = 0,
		AV2_BT_ANY,
		AV2_BT_NULL,
		AV2_BT_BOOL,
		AV2_BT_CHAR,
		AV2_BT_INT8,
		AV2_BT_UINT8,
		AV2_BT_INT16,
		AV2_BT_UINT16,
		AV2_BT_INT32,
		AV2_BT_UINT32,
		AV2_BT_INT64,
		AV2_BT_UINT64,
		AV2_BT_REAL32,
		AV2_BT_REAL64,
		AV2_BT_REAL128,
		AV2_BT_VECTOR,
		AV2_BT_MATRIX,
		AV2_BT_BYTES,
		AV2_BT_STRING,
		AV2_BT_TYPEID,
		AV2_BT_JUMPID,
		AV2_BT_CALLID,
	};

	constexpr bool isBoolean(BasicType const bt) {
		return bt == BasicType::AV2_BT_BOOL;
	}

	constexpr bool isSigned(BasicType const bt) {
		return (
			bt == BasicType::AV2_BT_INT8
		||	bt == BasicType::AV2_BT_INT16
		||	bt == BasicType::AV2_BT_INT32
		||	bt == BasicType::AV2_BT_INT64
		);
	}

	constexpr bool isUnsigned(BasicType const bt) {
		return (
			bt == BasicType::AV2_BT_UINT8
		||	bt == BasicType::AV2_BT_UINT16
		||	bt == BasicType::AV2_BT_UINT32
		||	bt == BasicType::AV2_BT_UINT64
		);
	}

	constexpr bool isInteger(BasicType const bt) {
		return (isSigned(bt) || isUnsigned(bt));
	}

	constexpr bool isReal(BasicType const bt) {
		return (
			bt == BasicType::AV2_BT_REAL32
		||	bt == BasicType::AV2_BT_REAL64
		||	bt == BasicType::AV2_BT_REAL128
		);
	}

	constexpr bool isTypeID(BasicType const bt) {
		return (bt == BasicType::AV2_BT_TYPEID);
	}

	constexpr bool isJumpID(BasicType const bt) {
		return (bt == BasicType::AV2_BT_JUMPID);
	}

	constexpr bool isCallID(BasicType const bt) {
		return (bt == BasicType::AV2_BT_CALLID);
	}

	constexpr bool isNumber(BasicType const bt) {
		return (isInteger(bt) || isReal(bt));
	}

	constexpr bool isString(BasicType const bt) {
		return (bt == BasicType::AV2_BT_STRING);
	}

	constexpr bool isCharacter(BasicType const bt) {
		return (bt == BasicType::AV2_BT_CHAR);
	}

	constexpr bool isText(BasicType const bt) {
		return (isCharacter(bt) || isString(bt));
	}

	constexpr bool isBytes(BasicType const bt) {
		return (bt == BasicType::AV2_BT_BYTES);
	}

	constexpr bool isVector(BasicType const bt) {
		return (bt == BasicType::AV2_BT_VECTOR);
	}

	constexpr bool isMatrix(BasicType const bt) {
		return (bt == BasicType::AV2_BT_MATRIX);
	}

	constexpr bool isVectorable(BasicType const bt) {
		return (isNumber(bt) || isVector(bt));
	}

	constexpr bool isAlgebraic(BasicType const bt) {
		return (isVectorable(bt) || isMatrix(bt));
	}

	struct [[CTL_FLAG_STRUCT(uint64)]] TypeFlags {
		uint64 isBasic:		1 = false;
		uint64 isNullable:	1 = false;
		uint64 isEmpty:		1 = false;
		uint64 hasNoResult:	1 = false;
		uint64 isArray:		1 = false;
		uint64 isValueType:	1 = false;
		uint64 isStructure:	1 = false;
		uint64 isDynamic:	1 = false;
		uint64 isCopyable:	1 = false;
		uint64 isProxy:		1 = false;
		uint64 isPointer:	1 = false;
		uint64 isFinal:		1 = false;
		uint64 isEnum:		1 = false;
		uint64 isFunction:	1 = false;
		CTL_FLAG_STRUCT_END(uint64);
	};

	static_assert(sizeof(TypeFlags) == sizeof(uint64), "Uh oh :/");

	struct Definition: Entry, Flagged<TypeFlags> {
		using Source = MemorySlice<byte>;

		bool canBecome(AtomicCell<Definition> const& type) const {
			if (type == base) return true;
			AtomicCell<Definition> current = base;
			if (!base) return false;
			do if (current == type) return true;
			while ((current = current->base));
			return false;
		}

		static void makeBasic(Definition& type);

		Nullable<BasicType>				basic;
		AtomicCell<Definition>			base		= nullptr;
		uint64							byteSize	= 0;
		uint64							alignment	= 1;
		List<AtomicCell<Definition>>	fields;

		Data::Value::ObjectType		meta;

		using Constructor	= Functor<void(Source&)>;
		using Destructor	= Functor<void(Source&)>;
		using Cloner		= Functor<void(Source&, Source const&)>;
		using Comparator	= Functor<int64(Source const&, Source const&)>;
		using Stringifier	= Functor<UTF8String(Object const&)>;

		Constructor	construct;
		Destructor	destruct;
		Cloner		copy;
		Comparator	compare;

		Stringifier	toStringInternal;
	};

	Definition::Constructor	constructorOf(BasicType const type);
	Definition::Destructor	destructorOf(BasicType const type);
	Definition::Cloner		clonerOf(BasicType const type);
	Definition::Comparator	comparatorOf(BasicType const type);
	Definition::Stringifier	stringifierOf(BasicType const type);
}

#endif
