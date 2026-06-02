#include "type.hpp"

using namespace Makai;
using namespace Makai::Anima::V2;
using namespace Makai::Anima::V2::Core;


template <class T>
static void castAndConstructImpl(Definition::Source& a) {
	if (!a) throw Makai::Error::NonexistentValue("Oops, hehe :3");
	MX::construct<T>(Cast::rewrite<ref<T>>(a.data()));
}

static Definition::Constructor doNotConstruct() {
	return {};
}

template <class T>
static Definition::Destructor castAndConstruct() {
	return {castAndConstructImpl<T>};
}

template <class T>
static Definition::Destructor proxyConstruct() {
	return {proxyConstructImpl<T>};
}

Definition::Constructor Core::constructorOf(BasicType const type) {
	switch (type) {
		case BasicType::AV2_BT_MATRIX:	return castAndConstruct<Matrix4x4>();
		case BasicType::AV2_BT_VECTOR:	return castAndConstruct<Vector4>();
		default: return doNotConstruct();
	}
}

template <class T>
static void castAndDestructImpl(ref<void> a) {
	if (!a) return;
	MX::destruct<T>(Cast::rewrite<ref<T>>(a));
}

template <class T>
static Definition::Destructor castAndDestruct() {
	return {castAndDestructImpl<T>};
}

static Definition::Destructor doNotDestruct() {
	return {};
}

Definition::Destructor Core::destructorOf(BasicType const type) {
	switch (type) {
		case BasicType::AV2_BT_MATRIX:	return castAndDestruct<Matrix4x4>();
		case BasicType::AV2_BT_VECTOR:	return castAndDestruct<Vector4>();
		default: return doNotDestruct();
	}
}

template <class T>
static void castAndCloneImpl(Definition::Source& a, Definition::Source const& b) {
	if (!a) throw Makai::Error::NonexistentValue("Oops, hehe :3");
	if (!b) throw Makai::Error::NonexistentValue("Oops, hehe :3");
	violate<T>(a.data()) = violate<T>(b.data());
}

template <class T>
static void proxyCloneImpl(Definition::Source& a, Definition::Source const& b) {
	if (b.size())
		MX::memmove(a.resize(b.size()).data(), b.data(), b.size());
	else a.free();
}

template <class T>
static Definition::Cloner castAndClone() {
	return {castAndCloneImpl<T>};
}

template <class T>
static Definition::Cloner proxyClone() {
	return {proxyCloneImpl<T>};
}

static Definition::Cloner doNotClone() {
	return {};
}

Definition::Cloner Core::clonerOf(BasicType const type) {
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
		case BasicType::AV2_BT_STRING:	return proxyClone<UTF8String>();
		case BasicType::AV2_BT_BYTES:	return proxyClone<Bytes<>>();
		case BasicType::AV2_BT_VECTOR:	return castAndClone<Vector4>();
		case BasicType::AV2_BT_TYPEID:	return castAndClone<TypeID>();
		case BasicType::AV2_BT_MATRIX:	return castAndClone<Matrix4x4>();
		default: return doNotClone();
	}
}

template <class T>
static int64 castAndCompareImpl(Definition::Source const& a, Definition::Source const& b) {
	if (!a) throw Makai::Error::NonexistentValue("Oops, hehe :3");
	if (!b) throw Makai::Error::NonexistentValue("Oops, hehe :3");
	return enumcast<StandardOrder>(violate<T>(a) <=> violate<T>(b));
}

template <class T>
static int64 proxyCompareImpl(Definition::Source const& a, Definition::Source const& b) {
	if (!a) throw Makai::Error::NonexistentValue("Oops, hehe :3");
	if (!b) throw Makai::Error::NonexistentValue("Oops, hehe :3");
	return MX::memcmp(a.data(), b.data(), (a.size() < b.size() ? a.size() : b.size()));
}

template <class T>
static Definition::Comparator castAndCompare() {
	return {castAndCompareImpl<T>};
}

template <class T>
static int64 primitiveCompareImpl(Definition::Source const& a, Definition::Source const& b) {
	return MX::memcmp(a, b, sizeof(T));
}

template <class T>
static Definition::Comparator primitiveCompare() {
	return {primitiveCompareImpl<T>};
}

static Definition::Comparator doNotCompare() {
	return {};
}

Definition::Comparator Core::comparatorOf(BasicType const type) {
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
		case BasicType::AV2_BT_STRING:	return proxyCompare<UTF8String>();
		case BasicType::AV2_BT_BYTES:	return proxyCompare<Bytes<>>();
		case BasicType::AV2_BT_VECTOR:	return castAndCompare<Vector4>();
		case BasicType::AV2_BT_TYPEID:	return castAndCompare<TypeID>();
		default: return doNotCompare();
	}
}

void Definition::makeBasic(Definition& type) {
	type.byteSize = 0;
	switch (*type.basic) {
		using enum BasicType;
		case AV2_BT_BOOL:		type.byteSize = sizeof(bool);				break;
		case AV2_BT_INT8:		type.byteSize = sizeof(int8);				break;
		case AV2_BT_UINT8:		type.byteSize = sizeof(uint8);				break;
		case AV2_BT_INT16:		type.byteSize = sizeof(int16);				break;
		case AV2_BT_UINT16:		type.byteSize = sizeof(uint16);				break;
		case AV2_BT_INT32:		type.byteSize = sizeof(int32);				break;
		case AV2_BT_UINT32:		type.byteSize = sizeof(uint32);				break;
		case AV2_BT_INT64:		type.byteSize = sizeof(int64);				break;
		case AV2_BT_UINT64:		type.byteSize = sizeof(uint64);				break;
		case AV2_BT_REAL32:		type.byteSize = sizeof(float32);			break;
		case AV2_BT_REAL64:		type.byteSize = sizeof(float64);			break;
		case AV2_BT_REAL128:	type.byteSize = sizeof(float128);			break;
		case AV2_BT_CHAR:		type.byteSize = sizeof(Makai::UTF8Char);	break;
		case AV2_BT_VECTOR:		type.byteSize = sizeof(Makai::Vector4);		break;
		case AV2_BT_MATRIX:		type.byteSize = sizeof(Makai::Matrix4x4);	break;
		case AV2_BT_STRING:		type.alignment = 4;							break;
		case AV2_BT_BYTES:		type.alignment = 1;							break;
		case AV2_BT_TYPEID:		type.byteSize = sizeof(TypeID);				break;
		case AV2_BT_VOID:		type.alignment = 0;							break;
		case AV2_BT_ANY:		type.alignment = 0;							break;
		case AV2_BT_NULL:		type.alignment = 0;							break;
		default: break;
	}
	type.construct	= constructorOf(type.basic);
	type.destruct	= destructorOf(type.basic);
	type.copy		= clonerOf(type.basic);
	type.compare	= comparatorOf(type.basic);
}
