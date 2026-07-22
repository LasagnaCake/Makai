#include "object.hpp"
#include "type.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

Object::~Object() {
	if (content.unique()) {
		if (!origin) return;
		if (!(origin->flags.isValueType))
			origin->destruct(*content);
	}
}

Object::Storage Object::as(AtomicCell<Definition> const& newType) const {
	if (type && type->canBecome(newType))
		return Object::create(*this, newType);
	else if (!type && origin && origin->canBecome(newType))
		return Object::create(*this, newType);
	else return null;
}

bool Object::changeType(AtomicCell<Definition> const& newType) {
	if (type && type->canBecome(newType)) {
		type = newType;
		return true;
	}
	else if (!type && origin && origin->canBecome(newType)) {
		type = newType;
		return true;
	}
	else return false;
}

Result<Object::Storage, Object::GetError> Object::getAtIndex(uint64 const index) const {
	auto const t = getType();
	if (!t) return GetError::AV2_COGE_NO_TYPE;
	if (!(t->flags.isArray || t->flags.isStructure))
		return GetError::AV2_COGE_TYPE_DOES_NOT_CONTAIN_FIELDS;
	MAKAILIB_DEBUGLN_FULL("Type (Size: ", t->fields.size(), " :: ", fields.size(), ") -> [", index, "]");
	if (t->flags.isStructure && (index >= t->fields.size()))
		return GetError::AV2_COGE_FIELD_DOES_NOT_EXIST;
	if (t->flags.isValueType) {
		if (t->flags.isStructure && !t->fields[index]->flags.isCopyable)
			return GetError::AV2_COGE_FIELD_IS_NOT_COPYABLE;
		if (t->flags.isArray && !t->base->flags.isCopyable)
			return GetError::AV2_COGE_FIELD_IS_NOT_COPYABLE;
		return cloneFrom(index);
	}
	if (index < fields.size()) {
		MAKAILIB_DEBUGLN_FULL("[", index, "] -> ", fields[index]->toDynamicValue().toFLOWString());
		return fields[index];
	}
	return GetError::AV2_COGE_FIELD_DOES_NOT_EXIST;
}

Object::SetError Object::setAtIndex(uint64 const index, Object::Storage const& value) {
	auto const t = getType();
	if (!t) return SetError::AV2_COSE_NO_TYPE;
	if (!(t->flags.isArray || t->flags.isStructure))
		return SetError::AV2_COSE_TYPE_DOES_NOT_CONTAIN_FIELDS;
	MAKAILIB_DEBUGLN_FULL("Type (Size:", t->fields.size(), ") -> [", index, "]");
	if (t->flags.isStructure && index >= t->fields.size())
		return SetError::AV2_COSE_FIELD_DOES_NOT_EXIST;
	if (t->flags.isValueType) {
		if (t->flags.isStructure) {
			if (index >= t->fields.size())
				return SetError::AV2_COSE_FIELD_DOES_NOT_EXIST;
			if (!t->fields[index]->flags.isCopyable)
				return SetError::AV2_COSE_FIELD_IS_NOT_COPYABLE;
		} else if (t->flags.isArray) {
			if (index >= count())
				return SetError::AV2_COSE_FIELD_DOES_NOT_EXIST;
			if (!t->base->flags.isCopyable)
				return SetError::AV2_COSE_FIELD_IS_NOT_COPYABLE;
		} else return SetError::AV2_COSE_TYPE_DOES_NOT_CONTAIN_FIELDS;
		MX::memcpy(addressAt(index), value->content->data(), value->getType()->byteSize);
		return SetError::AV2_COSE_OK;
	}
	if (index < fields.size()) {
		MAKAILIB_DEBUGLN_FULL("[", index, "] -> ", value->toDynamicValue().toFLOWString());
		fields[index] = value;
		MAKAILIB_DEBUGLN_FULL("[", index, "] -> ", 	fields[index]->toDynamicValue().toFLOWString());
		return SetError::AV2_COSE_OK;
	}
	return SetError::AV2_COSE_FIELD_DOES_NOT_EXIST;
}

Object::Storage Object::clone() const {
	if (origin->copy)
		return create(*this);
	return null;
}

Object::Storage Object::clone() {
	if (origin->copy)
		return create(*this);
	return null;
}

void Object::reserveFields(usize const count) {
	fields.reserve(count, null);
	if (fields.size() < count)
		throw Error::FailedAction("Failed to reserve fields!", CTL_CPP_PRETTY_SOURCE);
}

Object::Storage Object::cloneFrom(usize const index) const {
	if (index >= count()) return nullptr;
	if (isValueType()) {
		auto const addr = addressAt(index);
		auto const mem = AtomicCell<Memory>::create();
		if (isStructure() && isClonable()) {
			auto const t = origin->fields[index];
			mem->resize(t->byteSize);
			MX::memcpy(mem->data(), addr, t->byteSize);
			return create(mem, t, t);
		}
		if (isArray() && origin->base->flags.isCopyable) {
			mem->resize(origin->base->byteSize);
			MX::memcpy(mem->data(), addr, origin->base->byteSize);
			return create(mem, getType()->base, origin->base);
		}
	} else if (fields[index])
		return fields[index]->clone();
	return nullptr;
}

usize Object::count() const {
	if (!origin) return -1;
	if (!isValueType())
		return fields.size();
	if (!(content & content->size())) return 0;
	else if (isArray())
		return content->size() / origin->base->byteSize;
	else if (isStructure())
		return origin->fields.size();
	else return 1;
}

pointer Object::addressAt(usize index) const {
	if (isArray())
		return (content->data() + index * origin->byteSize);
	else if (isStructure()) {
		auto const fcount = origin->fields.size();
		ref<byte> addr = content->data();
		while (index > 0)
			addr += origin->fields[fcount - (--index)]->byteSize;
		return addr;
	} else return content->data();
}

AtomicCell<Definition> Object::getType() const {
	return type ? type : origin;
}

AtomicCell<Definition> Object::getCurrentType() const {
	return getType();
}

AtomicCell<Definition> Object::getOriginalType() const {
	return origin;
}
