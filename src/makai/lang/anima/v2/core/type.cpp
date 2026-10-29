#include "type.hpp"

using namespace Makai;
using namespace Makai::Anima::V2;
using namespace Makai::Anima::V2::Core;

String Core::asNameString(BasicType const type) {
	switch (type) {
		case Core::BasicType::AV2_BT_ANY:		return "any";	break;
		case Core::BasicType::AV2_BT_INT8:		return "i8";	break;
		case Core::BasicType::AV2_BT_INT16:		return "i16";	break;
		case Core::BasicType::AV2_BT_INT32:		return "i32";	break;
		case Core::BasicType::AV2_BT_INT64:		return "i64";	break;
		case Core::BasicType::AV2_BT_UINT8:		return "u8";	break;
		case Core::BasicType::AV2_BT_UINT16:	return "u16";	break;
		case Core::BasicType::AV2_BT_UINT32:	return "u32";	break;
		case Core::BasicType::AV2_BT_UINT64:	return "u64";	break;
		case Core::BasicType::AV2_BT_REAL32:	return "f32";	break;
		case Core::BasicType::AV2_BT_REAL64:	return "f64";	break;
		case Core::BasicType::AV2_BT_REAL128:	return "f128";	break;
		case Core::BasicType::AV2_BT_STRING:	return "str";	break;
		case Core::BasicType::AV2_BT_VECTOR:	return "vec";	break;
		case Core::BasicType::AV2_BT_MATRIX:	return "mat";	break;
		case Core::BasicType::AV2_BT_TYPEID:	return "type";	break;
		case Core::BasicType::AV2_BT_BYTES:		return "bin";	break;
		case Core::BasicType::AV2_BT_NULL:		return "nil";	break;
		case Core::BasicType::AV2_BT_VOID:		return "void";	break;
		case Core::BasicType::AV2_BT_BOOL:		return "bool";	break;
		case Core::BasicType::AV2_BT_CHAR:		return "char";	break;
		default: return "";
	}
}
