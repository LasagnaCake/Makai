#include "type.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

template <class T>
static Definition::Constructor castAndConstruct() {
	return {
		[] (auto a) {
			MX::construct<T>(Cast::rewrite<ref<T>>(a));
		}
	};
}

static Definition::Constructor doNotConstruct() {
	return {};
}

Definition::Constructor constructorOf(BasicType const type) {
	switch (type) {
		case BasicType::AV2_BT_STRING:	return castAndConstruct<UTF8String>();
		case BasicType::AV2_BT_BYTES:	return castAndConstruct<Bytes<>>();
		case BasicType::AV2_BT_MATRIX:	return castAndConstruct<Matrix4x4>();
		case BasicType::AV2_BT_VECTOR:	return castAndConstruct<Vector4>();
		default: return doNotConstruct();
	}
}
}

template <class T>
static Definition::Destructor castAndDestruct() {
	return {
		[] (auto a) {
			MX::destruct<T>(Cast::rewrite<ref<T>>(a));
		}
	};
}

static Definition::Destructor doNotDestruct() {
	return {};
}

Definition::Destructor destructorOf(BasicType const type) {
	switch (type) {
		case BasicType::AV2_BT_STRING:	return castAndDestruct<UTF8String>();
		case BasicType::AV2_BT_BYTES:	return castAndDestruct<Bytes<>>();
		case BasicType::AV2_BT_MATRIX:	return castAndDestruct<Matrix4x4>();
		case BasicType::AV2_BT_VECTOR:	return castAndDestruct<Vector4>();
		default: return doNotDestruct();
	}
}

template <class T>
static Definition::Cloner castAndClone() {
	return {
		[] (auto a, auto const b) {
			violate<T>(a) = violate<T>(b);
		}
	};
}

static Definition::Cloner doNotClone() {
	return {};
}

Definition::Cloner clonerOf(BasicType const type) {
	switch (type) {
		case BasicType::AV2_BT_BOOL:	return castAndClone<bool>();
		case BasicType::AV2_BT_INT8:	return castAndClone<int8>();
		case BasicType::AV2_BT_UINT8:	return castAndClone<uint8>();
		case BasicType::AV2_BT_INT16:	return castAndClone<int16>();
		case BasicType::AV2_BT_UINT16:	return castAndClone<uint16>();
		case BasicType::AV2_BT_INT32:	return castAndClone<int32>();
		case BasicType::AV2_BT_UINT32:	return castAndClone<uint32>();
		case BasicType::AV2_BT_INT64:	return castAndClone<int64>();
		case BasicType::AV2_BT_UINT64:	return castAndClone<uint64>();
		case BasicType::AV2_BT_REAL32:	return castAndClone<float32>();
		case BasicType::AV2_BT_REAL64:	return castAndClone<float64>();
		case BasicType::AV2_BT_REAL128:	return castAndClone<float128>();
		case BasicType::AV2_BT_CHAR:	return castAndClone<UTF8Char>();
		case BasicType::AV2_BT_STRING:	return castAndClone<UTF8String>();
		case BasicType::AV2_BT_BYTES:	return castAndClone<Bytes<>>();
		case BasicType::AV2_BT_VECTOR:	return castAndClone<Vector4>();
		case BasicType::AV2_BT_TYPEID:	return castAndClone<TypeID>();
		case BasicType::AV2_BT_MATRIX:	return castAndClone<Matrix4x4>();
		default: return doNotClone();
	}
}

template <class T>
static Definition::Comparator castAndCompare() {
	return {
		[] (auto const a, auto const b) -> bool {
			return enumcast<StandardOrder>(violate<T>(a) <=> violate<T>(b));
		}
	};
}

template <class T>
static Definition::Comparator primitiveCompare() {
	return {
		[] (auto const a, auto const b) -> bool {
			return MX::memcmp(a, b, sizeof(T));
		}
	};
}

static Definition::Comparator doNotCompare() {
	return {};
}

Definition::Comparator comparatorOf(BasicType const type) {
	switch (type) {
		case BasicType::AV2_BT_BOOL:	return primitiveCompare<bool>();
		case BasicType::AV2_BT_INT8:	return primitiveCompare<int8>();
		case BasicType::AV2_BT_UINT8:	return primitiveCompare<uint8>();
		case BasicType::AV2_BT_INT16:	return primitiveCompare<int16>();
		case BasicType::AV2_BT_UINT16:	return primitiveCompare<uint16>();
		case BasicType::AV2_BT_INT32:	return primitiveCompare<int32>();
		case BasicType::AV2_BT_UINT32:	return primitiveCompare<uint32>();
		case BasicType::AV2_BT_INT64:	return primitiveCompare<int64>();
		case BasicType::AV2_BT_UINT64:	return primitiveCompare<uint64>();
		case BasicType::AV2_BT_REAL32:	return primitiveCompare<float32>();
		case BasicType::AV2_BT_REAL64:	return primitiveCompare<float64>();
		case BasicType::AV2_BT_REAL128:	return primitiveCompare<float128>();
		case BasicType::AV2_BT_CHAR:	return castAndCompare<UTF8Char>();
		case BasicType::AV2_BT_STRING:	return castAndCompare<UTF8String>();
		case BasicType::AV2_BT_BYTES:	return castAndCompare<Bytes<>>();
		case BasicType::AV2_BT_VECTOR:	return castAndCompare<Vector4>();
		case BasicType::AV2_BT_TYPEID:	return castAndCompare<TypeID>();
		default: return doNotCompare();
	}
}

void Definition::makeBasic(Definition& type) {
	type.construct	= constructorOf(type.basic);
	type.destruct	= destructorOf(type.basic);
	type.copy		= clonerOf(type.basic);
	type.compare	= comparatorOf(type.basic);
}
