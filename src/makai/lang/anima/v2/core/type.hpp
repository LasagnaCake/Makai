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
		AV2_BT_TYPEID,
		AV2_BT_JUMPID,
		AV2_BT_VECTOR,
		AV2_BT_MATRIX,
		AV2_BT_BYTES,
		AV2_BT_STRING,
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

	struct Definition: Entry {
		using Source = MemorySlice<byte>;

		struct Flags {
			constexpr static uint64 const AV2_DF_BASIC			= 1 << 0;
			constexpr static uint64 const AV2_DF_NULLABLE		= 1 << 1;
			constexpr static uint64 const AV2_DF_EMPTY			= 1 << 2;
			constexpr static uint64 const AV2_DF_ARRAY			= 1 << 3;
			constexpr static uint64 const AV2_DF_VALUE			= 1 << 4;
			constexpr static uint64 const AV2_DF_STRUCTURE		= 1 << 5;
			constexpr static uint64 const AV2_DF_DYNAMIC		= 1 << 6;
			constexpr static uint64 const AV2_DF_CLONABLE		= 1 << 7;
			constexpr static uint64 const AV2_DF_PROXY			= 1 << 8;
			constexpr static uint64 const AV2_DF_NO_RESULT		= 1 << 9;
			constexpr static uint64 const AV2_DF_POINTER		= 1 << 10;
			constexpr static uint64 const AV2_DF_FINAL			= 1 << 11;
		};

		bool canBecome(Handle<Definition> const& type) const {
			if (type == base) return true;
			Handle<Definition> current = base;
			while ((current = current->base))
				if (current == type) return true;
			return false;
		}

		static void makeBasic(Definition& type);

		uint64						flags		= 0;
		Nullable<BasicType>			basic;
		Handle<Definition>			base		= nullptr;
		uint64						byteSize	= 0;
		uint64						alignment	= 1;
		List<Handle<Definition>>	fields;

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
