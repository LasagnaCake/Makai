#include "value.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

Object::~Object() {

}

Object::Storage Object::getAtIndex(uint64 const index) const {
	if (!(type && type->base)) return null;
	if (!(type->flags & Definition::Flags::AV2_DF_ARRAY))
		return null;
	if (type->flags & Definition::Flags::AV2_DF_VALUE) {
		if (!(type->flags & Definition::Flags::AV2_DF_CLONABLE)) return null;
		auto& arr = value.content;
		if (!(index < arr.size())) return null;
		auto const elem = new Object();
		auto const start	= arr.data() + index * type->byteSize;
		auto const sz		= type->byteSize;
		elem->value.content.invoke(sz);
		type->base->construct(start);
		type->base->clone.value().invoke(elem->value.content.data(), start);
		elem->value.destruct = [f = type->base->destruct] (Value& self) {
			f(self.content.data());
		};
		if (type->base->clone)
			elem->value.clone = [f = type->base->clone, sz] (Value& self) -> Value {
				Value v;
				v.content.invoke(sz);
				f(v.content.data(), e);
				v.destruct	= self.destruct;
				v.clone		= self.clone;
			};
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
	if (!(type && type->base)) return true;
	if (!(type->flags & Definition::Flags::AV2_DF_ARRAY))
		return true;
	if (type->flags & Definition::Flags::AV2_DF_VALUE) {
		// TODO: Value array set-by-index
	}
	if (!(index < fields.size()))
		return false;
	fields[index] = value;
	return true;
}

Object::Storage Object::clone() {
	return new Object(*this);
}
