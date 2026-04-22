#include "object.hpp"
#include "type.hpp"

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

Object::Storage Object::as(Instance<Definition> const& newType) const {
	if (type->canBecome(newType))
		return Object::create(*this, newType);
	else return null;
}

Object::Storage Object::getAtIndex(uint64 const index) const {
	if (!type) return null;
	if (!(isArray()|| isStructrure()))
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

Object::Storage Object::clone() const {
	if (type->copy)
		return create(*this);
	return null;
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
	if (!(content & content->size())) return 0;
	else if (isArray())
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
	return store->getAtIndex(index);
}

Object::Accessor const& Object::Accessor::set(Object::Storage const& value) const {
	store->setAtIndex(index, value);
	return *this;
}

Makai::Instance<Definition> Object::getCurrentType() const {
	return type;
}

Makai::Instance<Definition> Object::getOriginalType() const {
	return origin;
}
