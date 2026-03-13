#include "value.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

Object::~Object() {
}

Value::~Value() {
	if (content.unique()) {
		if (origin->flags & Definition::Flags::AV2_DF_VALUE) {
			for (usize i = 0; i < count(); ++i)
				origin->destruct(addressAt(i));
		} else origin->destruct(content->data());
	}
}

Object::Storage Object::getAtIndex(uint64 const index) const {
	if (!value) return null;
	if (!value->isArray())
		return null;
	if (value->isValueType()) {
		if (!value->isClonable()) return null;
		return new Object{.value = value->cloneFrom(index)};
	}
	return index < fields.size() ? fields[index] : null;
}

uint64 Object::resolveMethod(uint64 const id) {
	if (vtable.contains(id))
		return vtable[id];
	return id;
}

Object::Storage Object::field(uint64 const index) const {
	if (!(type)) return null;
	if (!(type->flags & Definition::Flags::AV2_DF_STRUCTURE))
		return null;
	return index < fields.size() ? fields[index] : null;
}

bool Object::setAtIndex(uint64 const index, Object::Storage const& value) {
	if (!(type && type->base)) return false;
	if (!(type->flags & Definition::Flags::AV2_DF_ARRAY))
		return false;
	if (type->flags & Definition::Flags::AV2_DF_VALUE) {
		if (index * this->type->byteSize >= this->value.content.size())
			return false;
		auto const v = this->value.content.data() + index * this->type->byteSize;
		if (!this->type->clone)
			return false;
		this->type->clone.value().invoke(v, value->value.content.data());
		return true;
	}
	if (!(index < fields.size()))
		return false;
	fields[index] = value;
	return true;
}

Object::Storage Object::clone() {
	if (value.clone)
		return new Object(*this);
	return null;
}
