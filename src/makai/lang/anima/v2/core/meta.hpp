#ifndef MAKAILIB_ANIMA_V2_CORE_META_H
#define MAKAILIB_ANIMA_V2_CORE_META_H

#include "type.hpp"
#include "method.hpp"
#include "object.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core::Meta {
	namespace Impl {
		template <class T>
		concept ValidType = requires {
			requires ARTType<T>;
			{T::convert} -> Makai::Type::Functional<
				Makai::Meta::If<
					Makai::Type::Void<T>,
					Object(Database<Definition>&),
					Object(Database<Definition>&, T const&)
					>
				>
			;
		};

		template <class T>
		consteval static auto specialHashOf() {
			if constexpr (Type::Equal<T, int8>)		return ConstHasher::hash("int8");
			if constexpr (Type::Equal<T, int16>)	return ConstHasher::hash("int16");
			if constexpr (Type::Equal<T, int32>)	return ConstHasher::hash("int32");
			if constexpr (Type::Equal<T, int64>)	return ConstHasher::hash("int64");
			if constexpr (Type::Equal<T, uint8>)	return ConstHasher::hash("uint8");
			if constexpr (Type::Equal<T, uint16>)	return ConstHasher::hash("uint16");
			if constexpr (Type::Equal<T, uint32>)	return ConstHasher::hash("uint32");
			if constexpr (Type::Equal<T, uint64>)	return ConstHasher::hash("uint64");
			if constexpr (Type::Equal<T, float32>)	return ConstHasher::hash("real32");
			if constexpr (Type::Equal<T, float64>)	return ConstHasher::hash("real64");
			if constexpr (Type::Equal<T, float128>)	return ConstHasher::hash("real128");
		}

		template <class T> struct ARTTI;

		template<> struct ARTTI<Void> {
			constexpr static auto const ART_HASH = ConstHasher::hash("void");

			static void construct(Object const&) {}

			static Object::Storage convert(Database<Definition>& db) {
				return Object::create(db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<void> {
			constexpr static auto const ART_HASH = ConstHasher::hash("void");

			static void construct(Object const&) {}

			static Object::Storage convert(Database<Definition>& db) {
				return Object::create(db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<Any> {
			constexpr static auto const ART_HASH = ConstHasher::hash("any");

			static Any construct(Object const& obj) {
				return {obj.clone()};
			}

			static Object::Storage convert(Database<Definition>& db, Any const&) {
				return Object::create(db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<nulltype> {
			constexpr static auto const ART_HASH = ConstHasher::hash("nil");

			static nulltype construct(Object const&) {
				return {};
			}

			static Object::Storage convert(Database<Definition>& db, nulltype const&) {
				return Object::create(db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<bool> {
			constexpr static auto const ART_HASH = ConstHasher::hash("bool");

			static bool construct(Object const& value) {
				return value.toValue<bool>();
			}

			static Object::Storage convert(Database<Definition>& db, bool const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Type::OneOf<char, UTF8Char, UTF32Char> T>
		struct ARTTI<T> {
			constexpr static auto const ART_HASH = ConstHasher::hash("char");

			static bool construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::SignedInteger T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = specialHashOf<T>();

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::UnsignedInteger T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = specialHashOf<T>();

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::Real T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = specialHashOf<T>();

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<Binary<>> {
			constexpr static auto const ART_HASH = ConstHasher::hash("bytes");

			static Binary<> construct(Object const& value) {
				return value.toValue<Binary<>>();
			}

			static Object::Storage convert(Database<Definition>& db, Binary<> const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::OneOf<String, UTF8String, UTF32String> T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = ConstHasher::hash("string");

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::OneOf<Vector2, Vector3, Vector4> T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = ConstHasher::hash("vector");

			static T construct(Object const& value) {
				return value.toValue<Vector4>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(Vector4(value), db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<Matrix4x4> {
			constexpr static auto const ART_HASH = ConstHasher::hash("matrix");

			static Matrix4x4 construct(Object const& value) {
				return value.toValue<Matrix4x4>();
			}

			static Object::Storage convert(Database<Definition>& db, Matrix4x4 const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<TypeID> {
			constexpr static auto const ART_HASH = ConstHasher::hash("type");

			static TypeID construct(Object const& value) {
				return value.toValue<TypeID>();
			}

			static Object::Storage convert(Database<Definition>& db, TypeID const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<Data::Value> {
			constexpr static auto const ART_HASH = ConstHasher::hash("any");

			static Data::Value construct(Object const& value) {
				return value.toValue<Data::Value>();
			}

			static Object::Storage convert(Database<Definition>& db, TypeID const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<ARTType T>
		struct ARTTI<T>: T {
			constexpr static auto const ART_HASH = ConstHasher::hash(T::ART_NAME);

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template <class... Types>
		struct ToObjectTuple {
			using Type = Tuple<Makai::Meta::If<false, Types, Object::Storage>...>;
		};

		template <class... Types>
		struct ListToTuple {
			using Type = typename ToObjectTuple<Types...>::Type;
			using ListType = List<Object::Storage>;

			constexpr static bool fits(ListType const& list) {
				return list.size() >= sizeof...(Types);
			}

			constexpr static Type make(ListType const& list) {
				return make(list, IntegerPack<sizeof...(Types)>());
			}

			template <usize... N>
			constexpr static Type make(ListType const& list, IndexTuple<N...>) {
				return {list[N]...};
			}
		};

		template <class... Types>
		struct ObjectTupleToArguments {
			using ObjectTupleType = ToObjectTuple<Types...>;
			using Type = Tuple<Types...>;

			constexpr static Type make(ObjectTupleType const& tup) {
				return make(tup, IntegerPack<sizeof...(Types)>());
			}

			template <usize... N>
			constexpr static Type make(ObjectTupleType const& tup, IndexTuple<N...>) {
				return {ARTTI<Makai::Meta::Select<N, Types...>>::convert(*tup.template get<N>())...};
			}
		};
	};

	template <class T>
	using ARTInfo = Impl::ARTTI<T>;

	template <class T>
	constexpr usize arthashof() {
		return ARTInfo<T>::ART_HASH;
	}

	template <class... Types>
	constexpr bool fits(List<Object::Storage> const& args) {
		return Impl::ListToTuple<Types...>::fits();
	}

	template <class... Types>
	constexpr Tuple<Types...> toArguments(List<Object::Storage> const& args) {
		return Impl::ObjectTupleToArguments<Types...>::make(
			Impl::ListToTuple<Types...>::make(args)
		);
	}
}

#endif
