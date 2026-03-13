#ifndef MAKAILIB_ANIMA_V2_CORE_VALUE_H
#define MAKAILIB_ANIMA_V2_CORE_VALUE_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct Value {
		using Storage = Instance<Value>;
		using Memory = MemorySlice<byte>;

		template <Type::NonVoid T>
		consteval static bool sized() {
			return (sizeof(T) >= sizeof(byte));
		}

		~Value();

		constexpr Value() noexcept {}

		constexpr Value(Value const& other, Instance<Definition> const& newType): content(other.content), type(newType), origin(other.origin) {
		}

		template <Makai::Type::Different<Value> T>
		constexpr Value(T const& v, Instance<Definition> const& info) {
			type = origin = info;
			content->invoke(origin->byteSize);
			origin->copy(content->data(), &v);
		}

		template <Makai::Type::Different<Value> T>
		constexpr Value(T&& v, Instance<Definition> const& info) {
			type = origin = info;
			content->invoke(origin->byteSize);
			origin->move(content->data(), &v);
		}

		constexpr Value(Value const& other) {
			if (other.origin->copy) {
				content->invoke(other.content->size());
				origin->copy.invoke(content->data(), other.content->data());
			}
		}

		Value(Value&& other) = default;

		ref<void const>	data() const		{return content->data();					}
		constexpr usize byteSize() const	{return origin->byteSize;					}

		Value as(Instance<Definition> const& newType) const {
			return Value(*this, newType);
		}

		constexpr Value& operator=(Value const& other) {
			if (other.type->copy)
				return operator=(Value(other));
			return *this;
		}

		Value& operator=(Value&&) = default;

		constexpr uint64 flags() const {
			return origin->flags;
		}

		void copyTo(usize const index, ref<void const> const data) {
			if (index < count())
				origin->copy(addressAt(index), data);
		}

		void moveTo(usize const index, ref<void const> const data) {
			if (index < count())
				origin->move(addressAt(index), data);
		}

		Storage cloneFrom(usize const index) const {
			if (index >= count()) return nullptr;
			auto const addr = atIndex(index);
			auto const mem = new Memory();
			if (isStructrure() && isClonable()) {
				mem->resize(origin->byteSize);
				origin->copy(mem->data(), addr);
				return new Value(mem, type, origin);
			}
			if (isArray() && origin->base->flags & Definition::Flags::AV2_DF_CLONABLE) {
				mem->resize(origin->base->byteSize);
				origin->copy(mem->data(), addr);
				return new Value(mem, type->base, origin->base);
			}
			return nullptr;
		}

		constexpr usize	count() const {
			if (isArray())
				return content->size() / origin->base->byteSize;
			else if (isStructrure())
				return origin->fields.size();
			else return 1;
		}

		bool isValueType() const {
			return (origin->flags & Definition::Flags::AV2_DF_VALUE);
		}

		bool isClonable() const {
			return (origin->flags & Definition::Flags::AV2_DF_CLONABLE);
		}

		bool isArray() const {
			return (origin->flags & Definition::Flags::AV2_DF_ARRAY);
		}

		bool isStructrure() const {
			return (origin->flags & Definition::Flags::AV2_DF_STRUCTURE);
		}

		pointer atIndex(usize const index) const {
			addressAt(index);
		}

	private:
		constexpr Value(
			Instance<Memory> const& content,
			Instance<Definition> const& type,
			Instance<Definition> const& origin
		): content(content), type(type), origin(origin) {
		}

		pointer addressAt(usize index) const {
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

		Instance<Memory>		content = new Memory();
		Instance<Definition>	type;
		Instance<Definition>	origin;
	};

	struct Object {
		using Storage = Instance<Object>;
		List<Storage>			fields;
		Value::Storage			value;
		Map<uint64, uint64>		vtable;

		~Object();

		uint64 resolveMethod(uint64 const id);

		Storage getAtIndex(uint64 const index) const;
		bool setAtIndex(uint64 const index, Storage const& value);
		Storage field(uint64 const id) const;

		Storage clone();
	};
}

#endif
