#ifndef MAKAILIB_ANIMA_V2_CORE_OBJECT_H
#define MAKAILIB_ANIMA_V2_CORE_OBJECT_H

#include "type.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core {
	template <class T>
	concept ARTType = requires (Object o) {
		requires Type::NonVoid<T>;
		sizeof(T) >= sizeof(byte);
		{T::ART_NAME}		-> Makai::Type::Equal<scstring>;
		{T::construct(o)}	-> Makai::Type::Equal<T>;
	};

	struct Object {
		using Storage = Instance<Object>;
		using Memory = MemorySlice<byte>;

		~Object();

		ref<void const>	data() const		{return content->data();	}
		constexpr usize byteSize() const	{return origin->byteSize;	}

		template <Makai::Type::Equal<nulltype> T>
		T toValue() const {
			if (!isNull())
				invalidCastError<T>("Mismatched types");
			return null;
		}

		template <Makai::Type::Equal<bool> T>
		T toValue() const {
			if (isNumber())
				return fromBasicNumber<bool>();
			if (!isBoolean())
				invalidCastError<T>("Mismatched types");
			return fromBasicNumber<T>();
		}

		template <Makai::Type::Number T>
		T toValue() const requires Makai::Type::Different<T, bool> {
			if (!isNumber())
				invalidCastError<T>("Mismatched types");
			return fromBasicNumber<T>();
		}

		template <Makai::Type::OneOf<String, UTF8String, UTF32String> T>
		T toValue() const {
			if (!isString())
				invalidCastError<T>("Mismatched types");
			if constexpr (Makai::Type::Equal<T, String>)
				return ref<UTF8String>(content->data())->toString();
			else return *ref<UTF8String>(content->data());
		}

		template <Makai::Type::Equal<Binary<>> T>
		T toValue() const {
			if (!isBytes())
				invalidCastError<T>("Mismatched types");
			return *ref<T>(content->data());
		}

		template <Makai::Type::OneOf<char, UTF8Char, UTF32Char> T>
		T toValue() const {
			if (!isCharacter())
				invalidCastError<T>("Mismatched types");
			return *ref<UTF8Char>(content->data());
		}

		template <Makai::Type::OneOf<Vector2, Vector3, Vector4> T>
		T toValue() const {
			if (isNumber())
				return toValue<float>();
			if (!isVector())
				invalidCastError<T>("Mismatched types");
			return *ref<Vector4>(content->data());
		}

		template <Makai::Type::Equal<Matrix4x4> T>
		T toValue() const {
			if (isNumber())
				return Matrix4x4::identity() * toValue<float>();
			if (isVector())
				return Matrix4x4::fromTranslation(toValue<Vector4>());
			if (!isMatrix())
				invalidCastError<T>("Mismatched types");
			return *ref<Matrix4x4>(content->data());
		}

		template <Makai::Type::Equal<TypeID> T>
		T toValue() const {
			if (!isTypeID())
				invalidCastError<T>("Mismatched types");
			return *ref<TypeID>(content->data());
		}

		template <ARTType T>
		T toValue() const {
			if (sizeof(T) != type->byteSize)
				invalidCastError<T>("Size mismatch");
			if (type->name != T::ART_NAME)
				invalidCastError<T>("Type mismatch");
			return T::construct(*this);
		}

		template <Type::Equal<Data::Value> T>
		T toValue() const {
			return toDynamicValue();
		}

		Object::Storage as(Instance<Definition> const& newType) const;

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

		bool isBoolean() const {
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_BOOL);
		}

		bool isValueType() const {
			return (origin->flags & Definition::Flags::AV2_DF_VALUE);
		}

		bool isClonable() const {
			return (origin->flags & Definition::Flags::AV2_DF_CLONABLE);
		}

		bool isAlgebraic() const {
			return isVector() || isMatrix();
		}

		bool isVectorable() const {
			return isNumber() || isVector();
		}

		bool isNumber() const {
			return isInteger() || isReal();
		}

		bool isInteger() const {
			return isSigned() || isUnsigned();
		}

		bool isSigned() const {
			if (!isBasic())
				return false;
			return (
				origin->basic == BasicType::AV2_BT_INT8
			||	origin->basic == BasicType::AV2_BT_INT16
			||	origin->basic == BasicType::AV2_BT_INT32
			||	origin->basic == BasicType::AV2_BT_INT64
			);
		}

		bool isUnsigned() const {
			if (!isBasic())
				return false;
			return (
				origin->basic == BasicType::AV2_BT_UINT8
			||	origin->basic == BasicType::AV2_BT_UINT16
			||	origin->basic == BasicType::AV2_BT_UINT32
			||	origin->basic == BasicType::AV2_BT_UINT64
			);
		}

		bool isReal() const {
			if (!isBasic())
				return false;
			return (
				origin->basic == BasicType::AV2_BT_REAL32
			||	origin->basic == BasicType::AV2_BT_REAL64
			||	origin->basic == BasicType::AV2_BT_REAL128
			);
		}

		bool isVector() const {
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_VECTOR);
		}

		bool isMatrix() const {
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_MATRIX);
		}

		bool isTypeID() const {
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_TYPEID);
		}

		bool isCharacter() const {
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_CHAR);
		}

		bool isString() const {
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_STRING);
		}

		bool isBytes() const {
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_BYTES);
		}

		bool isVoid() const {
			return origin->basic == BasicType::AV2_BT_VOID;
		}

		bool isNull() const {
			return origin->basic == BasicType::AV2_BT_NULL;
		}

		bool isArray() const {
			return (origin->flags & Definition::Flags::AV2_DF_ARRAY);
		}

		bool isStructrure() const {
			return (origin->flags & Definition::Flags::AV2_DF_STRUCTURE);
		}

		bool isBasic() const {
			return (origin->flags & Definition::Flags::AV2_DF_BASIC);
		}

		Data::Value toDynamicValue() const {
			if (!isBasic()) return Data::Value::undefined();
			if (isNull())		return toValue<nulltype>();
			if (isBoolean())	return toValue<bool>();
			if (isUnsigned())	return toValue<uint64>();
			if (isSigned())		return toValue<int64>();
			if (isNumber())		return toValue<double>();
			if (isVectorable())	return toValue<Vector4>();
			if (isString())		return toValue<String>();
			if (isBytes())		return toValue<Bytes<>>();
			return Data::Value::undefined();
		}

		Ordered::OrderType compareWith(Storage const& other) const {
			if (!other) return Ordered::Order::GREATER;
			if (!count())
				return (!other->count()) ? Ordered::Order::EQUAL : Ordered::Order::LESS;
			if (!type->compare)
				return (!other->type->compare) ? Ordered::Order::EQUAL : Ordered::Order::LESS;
			if ((type == other->type) || type->canBecome(other->type))
				return StandardOrder(type->compare(content->data(), other->content->data()).value());
			return Ordered::Order::UNORDERED;
		}

		Storage getAtIndex(uint64 const index) const;
		bool setAtIndex(uint64 const index, Storage const& value);

		Storage clone();
		Storage clone() const;

		struct Accessor {
			Accessor const& operator=(Storage const& value) const	{return set(value);	}
			operator Storage() const								{return get();		}

			Storage			get() const;
			Accessor const&	set(Storage const& value) const;

			Storage source() const;

			Accessor() = default;

			Accessor(Accessor const&) = default;
			Accessor(Accessor&&) = default;

		private:
			friend struct Object;

			usize	index;
			Storage store;

			Accessor(usize const& index, Storage const& value): index(index), store(value) {}
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

		Instance<Definition>	getCurrentType();
		Instance<Definition>	getOriginalType();

		template <class T> explicit operator T() const {return toValue<T>();}

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
		T fromBasicNumber() const {
			switch (*origin->basic) {
				using enum BasicType;
				case AV2_BT_BOOL: return *ref<bool>(content->data());
				case AV2_BT_INT8: return *ref<int8>(content->data());
				case AV2_BT_INT16: return *ref<int16>(content->data());
				case AV2_BT_INT32: return *ref<int32>(content->data());
				case AV2_BT_INT64: return *ref<int64>(content->data());
				case AV2_BT_UINT8: return *ref<uint8>(content->data());
				case AV2_BT_UINT16: return *ref<uint16>(content->data());
				case AV2_BT_UINT32: return *ref<uint32>(content->data());
				case AV2_BT_UINT64: return *ref<uint64>(content->data());
				case AV2_BT_REAL32: return *ref<float>(content->data());
				case AV2_BT_REAL64: return *ref<double>(content->data());
				case AV2_BT_REAL128: return *ref<long double>(content->data());
				default:
				invalidCastError<T>("Type mismatch");
			}
		}

		template <class T>
		[[noreturn]]
		void invalidCastError(String const& reason) const {
			throw Error::InvalidCast(
				"Could not convert [" + origin->name + "] to [" + nameof<T>() + "]!",
				reason,
				CTL_CPP_PRETTY_SOURCE
			);
		}

		List<Storage>			fields;
		Instance<Memory>		content = new Memory();
		Instance<Definition>	type;
		Instance<Definition>	origin;
	};

	constexpr Data::Value decay(Object::Storage const& val) {
		if (!val) return Data::Value::undefined();
		return val->toDynamicValue();
	}

	constexpr Data::Value decay(Any const& any) {
		return decay(any.value);
	}

	constexpr Data::Value operator*(Any const& any) {
		return decay(any);
	}
}

#endif
