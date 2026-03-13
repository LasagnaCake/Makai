#ifndef MAKAILIB_ANIMA_V2_CORE_OBJECT_H
#define MAKAILIB_ANIMA_V2_CORE_OBJECT_H

#include "type.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core {
	template <class T>
	concept ARTType = requires (Object o) {
		Type::NonVoid<T>;
		sizeof(T) >= sizeof(byte);
		{T::ART_NAME}		-> Makai::Type::Equal<scstring>;
		{T::construct(o)}	-> Makai::Type::Equal<T>;
	};

	struct Object {
		using Storage = Instance<Object>;
		using Memory = MemorySlice<byte>;

		Map<uint64, uint64>		vtable;

		~Object();

		ref<void const>	data() const		{return content->data();	}
		constexpr usize byteSize() const	{return origin->byteSize;	}

		template <Makai::Type::Equal<bool> T>
		T toValue() const {
			if (isArray())
				invalidCastError<T>("Type is array");
			if (origin->basic != BasicType::AV2_BT_BOOL)
				invalidCastError<T>("Type mismatch");
			return *ref<bool>(data());
		}

		template <Makai::Type::Number T>
		T toValue() const {
			if (isArray())
				invalidCastError<T>("Type is array");
			switch (*origin->basic) {
				using enum BasicType;
				case AV2_BT_INT: return *ref<int64>(content->data());
				case AV2_BT_UINT: return *ref<uint64>(content->data());
				case AV2_BT_REAL: return *ref<double>(content->data());
				default:
				invalidCastError<T>("Type mismatch");
			}
		}

		template <Makai::Type::OneOf<String, UTF8String> T>
		T toValue() const {
			if (isArray())
				invalidCastError<T>("Type is array");
			if (origin->basic != BasicType::AV2_BT_STRING)
				invalidCastError<T>("Type mismatch");
			return *ref<UTF8String>(data());
		}

		template <Makai::Type::OneOf<Binary<>, Vector4> T>
		Binary<> toValue() const {
			if (isArray())
				invalidCastError<T>("Type is array");
			if (origin->basic != BasicType::AV2_BT_BYTES)
				invalidCastError<T>("Type mismatch");
			return *ref<T>(data());
		}

		template <ARTType T>
		T toValue() const {
			if (isArray())
				invalidCastError<T>("Type is array");
			if (sizeof(T) != origin->byteSize)
				invalidCastError<T>("Size mismatch");
			if (String(T::ART_NAME) != origin->name)
				invalidCastError<T>("Type mismatch");
			return T::construct(*this);
		}

		Object as(Instance<Definition> const& newType) const {
			return Object(*this, newType);
		}

		constexpr Object& operator=(Object const& other) {
			if (other.type->copy)
				return operator=(Object(other));
			return *this;
		}

		constexpr uint64 flags() const {
			return origin->flags;
		}

		void copyTo(usize const index, ref<void const> const data) {
			if (index < count())
				origin->copy(addressAt(index), data);
		}

		Storage cloneFrom(usize const index) const;

		usize count() const;

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

		Storage getAtIndex(uint64 const index) const;
		bool setAtIndex(uint64 const index, Storage const& value);

		uint64 resolveMethod(uint64 const id);

		Storage clone();

		struct Accessor {
			Accessor& operator=(Storage const& value) const	{return set(value);	}
			operator Storage() const						{return get();		}

			Storage		get() const;
			Accessor&	set(Storage const& value) const;

			Storage source() const;

			Accessor() = default;

			Accessor(Accessor const&) = default;
			Accessor(Accessor&&) = default;

		private:
			friend struct Object;

			usize	index;
			Storage value;

			Accessor(usize const& index, Storage const& value): index(index), value(value) {}
		};

		Accessor	at(uint64 const index)			{return {index, this};		}
		Storage		at(uint64 const index) const	{return getAtIndex(index);	}

		Accessor	operator[](uint64 const index)			{return at(index);	}
		Storage		operator[](uint64 const index) const	{return at(index);	}

		static Storage create() {
			return new Object();
		}

		static Storage create(Instance<Definition> const& type) {
			return new Object(type);
		}

		static Storage create(Object const& other, Instance<Definition> const& newType) {
			return new Object(other, newType);
		}

		template <Makai::Type::Different<Object> T>
		static Storage create(T const& val, Instance<Definition> const& info) {
			return new Object(val, info);
		}

		static Storage create(Object const& other) {
			return new Object(other);
		}

		static Storage create(
			Instance<Memory> const& content,
			Instance<Definition> const& type,
			Instance<Definition> const& origin
		) {
			return new Object(content, type, origin);
		}

		Object(Object&&)			= default;
		Object& operator=(Object&&)	= default;

	private:
		friend Storage;

		constexpr Object() noexcept {}

		constexpr Object(
			Instance<Definition> const& type
		): type(type), origin(type) {
		}

		constexpr Object(
			Object const& other,
			Instance<Definition> const& newType
		): content(other.content), type(newType), origin(other.origin) {
		}

		template <Makai::Type::Different<Object> T>
		constexpr Object(T const& v, Instance<Definition> const& info) {
			type = origin = info;
			content->invoke(origin->byteSize);
			origin->copy(content->data(), &v);
		}

		constexpr Object(Object const& other): type(other.type), origin(other.type) {
			if (type->copy) {
				content->invoke(type->byteSize);
				type->copy.invoke(content->data(), other.content->data());
			}
		}

		constexpr Object(
			Instance<Memory> const& content,
			Instance<Definition> const& type,
			Instance<Definition> const& origin
		): content(content), type(type), origin(origin) {}

		pointer addressAt(usize index) const;

		template <class T>
		[[noreturn]]
		void invalidCastError(String const& reason) const {
			throw Error::InvalidCast(
				"Could not convert [" + nameof<T>() + "] to [" + origin->name + "]!",
				reason,
				CTL_CPP_PRETTY_SOURCE
			);
		}

		List<Storage>			fields;
		Instance<Memory>		content = new Memory();
		Instance<Definition>	type;
		Instance<Definition>	origin;
	};
}

#endif
