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

Object::Storage Object::as(Instance<Definition> const& newType) const {
	if (type && type->canBecome(newType))
		return Object::create(*this, newType);
	else if (!type && origin && origin->canBecome(newType))
		return Object::create(*this, newType);
	else return null;
}

bool Object::changeType(Instance<Definition> const& newType) {
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

Object::Storage Object::getAtIndex(uint64 const index) const {
	if (!getType()) return null;
	if (!(isArray()|| isStructure()))
		return null;
	if (isValueType()) {
		if (!isClonable())
			return null;
		return cloneFrom(index);
	}
	return index < fields.size() ? fields[index] : null;
}

Object::SetError Object::setAtIndex(uint64 const index, Object::Storage const& value) {
	auto const t = getType();
	if (!t) return SetError::AV2_COSE_NO_TYPE;
	if (!canHaveFields())
		return SetError::AV2_COSE_TYPE_DOES_NOT_CONTAIN_FIELDS;
	if (isValueType()) {
		if (isArray()) {
			if (t->fields.size() <= index)
				return SetError::AV2_COSE_FIELD_DOES_NOT_EXIST;
			if (!t->fields[index]->copy)
				return SetError::AV2_COSE_FIELD_IS_NOT_COPYABLE;
		} else if (isStructure()) {
			if (count() <= index)
				return SetError::AV2_COSE_FIELD_DOES_NOT_EXIST;
			if (!t->base->copy)
				return SetError::AV2_COSE_FIELD_IS_NOT_COPYABLE;
		} else return SetError::AV2_COSE_TYPE_DOES_NOT_CONTAIN_FIELDS;
		MX::memcpy(addressAt(index), value->content->data(), value->getType()->byteSize);
		return SetError::AV2_COSE_OK;
	}
	if (index < fields.size()) {
		fields[index] = value;
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
		auto const mem = new Memory();
		if (isStructure() && isClonable()) {
			auto const t = origin->fields[index];
			mem->resize(t->byteSize);
			MX::memcpy(mem->data(), addr, t->byteSize);
			return create(mem, t.raw(), t.raw());
		}
		if (isArray() && origin->base->flags.isCopyable) {
			mem->resize(origin->base->byteSize);
			MX::memcpy(mem->data(), addr, origin->base->byteSize);
			return create(mem, getType()->base.raw(), origin->base.raw());
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

Object::Storage Object::Accessor::get() const {
	return store->getAtIndex(index);
}

Object::Accessor const& Object::Accessor::set(Object::Storage const& value) const {
	store->setAtIndex(index, value);
	return *this;
}

Makai::Handle<Definition> Object::getType() const {
	return type ? type.asWeak() : origin.asWeak();
}

Makai::Handle<Definition> Object::getCurrentType() const {
	return getType().asWeak();
}

Makai::Handle<Definition> Object::getOriginalType() const {
	return origin.asWeak();
}
