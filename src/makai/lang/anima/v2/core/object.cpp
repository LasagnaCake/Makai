#include "object.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

Object::~Object() {
	if (content.unique()) {
		if (origin->flags & Definition::Flags::AV2_DF_VALUE) {
			for (usize i = 0; i < count(); ++i)
				origin->destruct(addressAt(i));
		} else origin->destruct(content->data());
	}
}

Object::Storage Object::getAtIndex(uint64 const index) const {
	if (!type) return null;
	if (!isArray())
		return null;
	if (isValueType()) {
		if (!isClonable())
			return null;
		return cloneFrom(index);
	}
	return index < fields.size() ? fields[index] : null;
}

bool Object::setAtIndex(uint64 const index, Object::Storage const& value) {
	if (!type) return false;
	if (isValueType()) {
		if (!(index > count() && type->copy))
			return false;
		type->copy(addressAt(index), value->content->data());
		return true;
	}
	if (!(index < fields.size()))
		return false;
	fields[index] = value;
	return true;
}

uint64 Object::resolveMethod(uint64 const id) {
	if (vtable.contains(id))
		return vtable[id];
	return id;
}

Object::Storage Object::clone() {
	if (type->copy)
		return create(*this);
	return null;
}

Object::Storage Object::cloneFrom(usize const index) const {
	if (index >= count()) return nullptr;
	auto const addr = addressAt(index);
	auto const mem = new Memory();
	if (isStructrure() && isClonable()) {
		mem->resize(origin->byteSize);
		origin->copy(mem->data(), addr);
		return create(mem, type, origin);
	}
	if (isArray() && origin->base->flags & Definition::Flags::AV2_DF_CLONABLE) {
		mem->resize(origin->base->byteSize);
		origin->copy(mem->data(), addr);
		return create(mem, type->base, origin->base);
	}
	return nullptr;
}


usize Object::count() const {
	if (isArray())
		return content->size() / origin->base->byteSize;
	else if (isStructrure())
		return origin->fields.size();
	else return 1;
}

pointer Object::addressAt(usize index) const {
	if (isArray())
		return (content->data() + index * origin->byteSize);
	else if (isStructrure()) {
		auto const fcount = origin->fields.size();
		ref<byte> addr = content->data();
		while (index > 0)
			addr += origin->fields[fcount - (--index)]->byteSize;
		return addr;
	} else return content->data();
}

Object::Storage Object::Accessor::get() const {
	return value->getAtIndex(index);
}

Object::Accessor const& Object::Accessor::set(Object::Storage const& value) const {
	value->setAtIndex(index, value);
	return *this;
}
