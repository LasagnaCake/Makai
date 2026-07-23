#ifndef MAKAILIB_ANIMA_V2_CORE_OBJECT_H
#define MAKAILIB_ANIMA_V2_CORE_OBJECT_H

#include "type.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core {
	template <class T>
	concept ARTType = requires (Object o) {
		requires Type::NonVoid<T>;
		sizeof(T) >= sizeof(byte);
		{T::ART_HASH}		-> Makai::Type::Equal<uint64>;
		{T::construct(o)}	-> Makai::Type::Equal<T>;
	};

	struct Object {
		using Storage = AtomicCell<Object>;
		using Memory = MemorySlice<byte>;

		~Object();

		pointer			data()				{return content->data();	}
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

		template <Makai::Type::OneOf<UTF8String, UTF32String> T>
		T toValue() const {
			if (!isString())
				invalidCastError<T>("Mismatched types");
			DEBUGLN("Fetching string [SIZE: ", content->size() / sizeof(UTF8Char), "]...");
			return UTF8String(cref<UTF8Char>(content->data()), content->size() / sizeof(UTF8Char));
		}

		template <Makai::Type::OneOf<String> T>
		T toValue() const {
			if (!isString())
				invalidCastError<T>("Mismatched types");
			else return toValue<UTF8String>().toString();
		}

		template <Makai::Type::Equal<Binary<>> T>
		T toValue() const {
			if (!isBytes())
				invalidCastError<T>("Mismatched types");
			return Bytes<>(content->data(), content->size());
		}

		template <Makai::Type::OneOf<char, UTF8Char, UTF32Char> T>
		T toValue() const {
			if (!isCharacter())
				invalidCastError<T>("Mismatched types");
			return *cref<UTF8Char>(content->data());
		}

		template <Makai::Type::OneOf<Vector2, Vector3, Vector4> T>
		T toValue() const {
			if (isNumber())
				return toValue<float>();
			if (!isVector())
				invalidCastError<T>("Mismatched types");
			return *cref<Vector4>(content->data());
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

		template <Makai::Type::Equal<JumpID> T>
		T toValue() const {
			if (!isJumpID())
				invalidCastError<T>("Mismatched types");
			return *ref<JumpID>(content->data());
		}

		template <Makai::Type::Equal<CallID> T>
		T toValue() const {
			if (!isCallID())
				invalidCastError<T>("Mismatched types");
			return *ref<CallID>(content->data());
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

		Object::Storage as(AtomicCell<Definition> const& newType) const;

		bool changeType(AtomicCell<Definition> const& newType);

		Object& operator=(Object const& other);

		constexpr TypeFlags flags() const {
			if (!origin) return {};
			return origin->flags;
		}

		Storage cloneFrom(usize const index) const;

		usize count() const;

		constexpr bool isBoolean() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_BOOL);
		}

		constexpr bool isValueType() const {
			if (!origin) return false;
			return (origin->flags.isValueType);
		}

		constexpr bool isClonable() const {
			if (!origin) return false;
			return (origin->flags.isCopyable);
		}

		constexpr bool isEmptyType() const {
			if (!origin) return true;
			return (origin->flags.hasNoResult);
		}

		constexpr bool isAlgebraic() const {
			return isVector() || isMatrix();
		}

		constexpr bool isVectorable() const {
			return isNumber() || isVector();
		}

		constexpr bool isNumber() const {
			return isInteger() || isReal();
		}

		constexpr bool isNonBoolNumber() const {
			return isNonBoolInteger() || isReal();
		}

		constexpr bool isNonBoolUnsigned() const {
			return (!isBoolean()) && isUnsigned();
		}

		constexpr bool isNonBoolInteger() const {
			return isSigned() or isNonBoolUnsigned();
		}

		constexpr bool isInteger() const {
			return isSigned() or isUnsigned();
		}

		constexpr bool isSigned() const {
			if (!isBasic())
				return false;
			return (
				origin->basic == BasicType::AV2_BT_INT8
			||	origin->basic == BasicType::AV2_BT_INT16
			||	origin->basic == BasicType::AV2_BT_INT32
			||	origin->basic == BasicType::AV2_BT_INT64
			);
		}

		constexpr bool isUnsigned() const {
			if (!isBasic())
				return false;
			return (
				origin->basic == BasicType::AV2_BT_BOOL
			||	origin->basic == BasicType::AV2_BT_UINT8
			||	origin->basic == BasicType::AV2_BT_UINT16
			||	origin->basic == BasicType::AV2_BT_UINT32
			||	origin->basic == BasicType::AV2_BT_UINT64
			);
		}

		constexpr bool isReal() const {
			if (!isBasic())
				return false;
			return (
				origin->basic == BasicType::AV2_BT_REAL32
			||	origin->basic == BasicType::AV2_BT_REAL64
			||	origin->basic == BasicType::AV2_BT_REAL128
			);
		}

		constexpr bool isVector() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_VECTOR);
		}

		constexpr bool isMatrix() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_MATRIX);
		}

		constexpr bool isTypeID() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_TYPEID);
		}

		constexpr bool isJumpID() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_JUMPID);
		}

		constexpr bool isCallID() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_CALLID);
		}

		constexpr bool isCharacter() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_CHAR);
		}

		constexpr bool isString() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_STRING);
		}

		constexpr bool isBytes() const {
			if (!origin) return false;
			if (!isBasic())
				return false;
			return (origin->basic == BasicType::AV2_BT_BYTES);
		}

		constexpr bool isVoid() const {
			if (!origin) return true;
			return origin->basic == BasicType::AV2_BT_VOID;
		}

		constexpr bool isNull() const {
			if (!origin) return true;
			return origin->basic == BasicType::AV2_BT_NULL;
		}

		constexpr bool isArray() const {
			if (!origin) return false;
			return (origin->flags.isArray);
		}

		constexpr bool isStructure() const {
			if (!origin) return false;
			return (origin->flags.isStructure);
		}

		constexpr bool canHaveFields() const {
			return isArray() or isStructure();
		}

		constexpr bool isBasic() const {
			if (!origin) return false;
			return (origin->flags.isBasic);
		}

		Data::Value toDynamicValue() const {
			if (!content->size()) return Data::Value::undefined();
			if (!isBasic()) 	return Data::Value::undefined();
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
				return StandardOrder(type->compare(*content, *other->content).value());
			return Ordered::Order::UNORDERED;
		}

		enum class SetError: uint8 {
			AV2_COSE_OK,
			AV2_COSE_NO_TYPE,
			AV2_COSE_TYPE_DOES_NOT_CONTAIN_FIELDS,
			AV2_COSE_FIELD_IS_NOT_COPYABLE,
			AV2_COSE_FIELD_DOES_NOT_EXIST,
		};

		enum class GetError: uint8 {
			AV2_COGE_NO_TYPE,
			AV2_COGE_TYPE_DOES_NOT_CONTAIN_FIELDS,
			AV2_COGE_FIELD_IS_NOT_COPYABLE,
			AV2_COGE_FIELD_DOES_NOT_EXIST,
		};

		Result<Storage, GetError> getAtIndex(uint64 const index) const;

		SetError setAtIndex(uint64 const index, Storage const& value);

		Storage clone()			const;
		Storage shallowClone()	const;

		void reserveFields(usize const count);

		static Storage create() {
			return Storage::create();
		}

		static Storage create(AtomicCell<Definition> const& type) {
			return Storage::create(type);
		}

		static Storage create(Object const& other, AtomicCell<Definition> const& newType) {
			return Storage::create(other, newType);
		}

		template <Makai::Type::Different<Object> T>
		static Storage create(T const& val, AtomicCell<Definition> const& info) {
			return Storage::create(val, info);
		}

		static Storage create(Object const& other) {
			return Storage::create(other);
		}

		static Storage create(Object const& other, nulltype) {
			return Storage::create(other, null);
		}

		static Storage create(
			AtomicCell<Memory> const& content,
			AtomicCell<Definition> const& type,
			AtomicCell<Definition> const& origin
		) {
			return Storage::create(content, type, origin);
		}

		Object(Object&&)			= default;
		Object& operator=(Object&&)	= default;

		AtomicCell<Definition>	getType()	const;

		AtomicCell<Definition>	getCurrentType()	const;
		AtomicCell<Definition>	getOriginalType()	const;

		template <class T> explicit operator T() const {return toValue<T>();}

	private:
		friend Storage;

		constexpr Object() noexcept {}

		constexpr Object(
			AtomicCell<Definition> const& type
		): type(type), origin(type) {
			if (type->flags.isValueType)
				content = content.create();
		}

		constexpr Object(
			Object const& other,
			AtomicCell<Definition> const& newType
		): content(other.content), type(newType), origin(other.origin) {
		}

		template <Makai::Type::Different<Object> T>
		constexpr Object(T const& v, AtomicCell<Definition> const& info): Object(info) {
			content = content.create();
			content->invoke(origin->byteSize);
			DEBUGLN("Object Type: ", type->hash);
			if constexpr (Type::OneOf<T, String, UTF8String, UTF32String>) {
				UTF8String const us = v;
				content->resize(us.size() * sizeof(UTF8Char));
				MX::memmove<UTF8Char>((ref<UTF8Char>)content->data(), us.data(), us.size());
			} else if constexpr (Type::Equal<T, Bytes<>>) {
				content->resize(v.size());
				MX::memmove(content->data(), v.data(), v.size());
			} else if constexpr (Type::OneOf<T, char, UTF8Char, UTF32Char>) {
				initialize<UTF8Char>(v);
			} else initialize<T>(v);
		}

		template <Makai::Type::Different<Object> T>
		void initialize(T const& v) {
			if (sizeof(v) < origin->byteSize)
				throw Error::FailedAction(
					"Origin type is too small to contain desired type!",
					CTL_CPP_PRETTY_SOURCE
				);
			MX::construct(ref<T>(content->data()), v);
		}

		Object(Object const& other);

		Object(Object const& other, nulltype);

		constexpr Object(
			AtomicCell<Memory> const& content,
			AtomicCell<Definition> const& type,
			AtomicCell<Definition> const& origin
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
				"Could not convert [" + toString(origin->hash) + "] to [" + nameof<T>() + "]!",
				reason,
				CTL_CPP_PRETTY_SOURCE
			);
		}

		List<Storage>			fields;
		AtomicCell<Memory>		content;
		AtomicCell<Definition>	type;
		AtomicCell<Definition>	origin;
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
