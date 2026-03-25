#ifndef MAKAILIB_ANIMA_V2_CORE_META_H
#define MAKAILIB_ANIMA_V2_CORE_META_H

#include "type.hpp"
#include "method.hpp"
#include "object.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core::Meta {
	struct Void	{};

	struct Any	{};

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
		consteval static cstring specialNameOf() {
			if constexpr (Type::Equal<T, int8>)		return "int8";
			if constexpr (Type::Equal<T, int16>)	return "int16";
			if constexpr (Type::Equal<T, int32>)	return "int32";
			if constexpr (Type::Equal<T, int64>)	return "int64";
			if constexpr (Type::Equal<T, uint8>)	return "uint8";
			if constexpr (Type::Equal<T, uint16>)	return "uint16";
			if constexpr (Type::Equal<T, uint32>)	return "uint32";
			if constexpr (Type::Equal<T, uint64>)	return "uint64";
			if constexpr (Type::Equal<T, float32>)	return "real32";
			if constexpr (Type::Equal<T, float64>)	return "real64";
			if constexpr (Type::Equal<T, float128>)	return "real128";
		}

		template <class T> struct ARTTI;

		template<> struct ARTTI<Void> {
			constexpr static scstring ART_NAME = "void";

			static void construct(Object const&) {}

			static Object::Storage convert(Database<Definition>& db) {
				return Object::create(db.byName(ART_NAME).front());
			}
		};

		template<> struct ARTTI<Any> {
			constexpr static scstring ART_NAME = "any";

			static Any construct(Object const&) {
				return {};
			}

			static Object::Storage convert(Database<Definition>& db, Any const&) {
				return Object::create(db.byName(ART_NAME).front());
			}
		};

		template<> struct ARTTI<nulltype> {
			constexpr static scstring ART_NAME = "nil";

			static nulltype construct(Object const&) {
				return {};
			}

			static Object::Storage convert(Database<Definition>& db, nulltype const&) {
				return Object::create(db.byName(ART_NAME).front());
			}
		};

		template<> struct ARTTI<bool> {
			constexpr static scstring ART_NAME = "bool";

			static bool construct(Object const& value) {
				return value.toValue<bool>();
			}

			static Object::Storage convert(Database<Definition>& db, bool const& value) {
				return Object::create(value, db.byName(ART_NAME).front());
			}
		};

		template<Type::OneOf<char, UTF8Char, UTF32Char> T>
		struct ARTTI<T> {
			constexpr static scstring ART_NAME = "char";

			static bool construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, bool const& value) {
				return Object::create(value, db.byName(ART_NAME).front());
			}
		};

		template<Makai::Type::SignedInteger T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = specialNameOf<T>();

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byName(ART_NAME).front());
			}
		};

		template<Makai::Type::UnsignedInteger T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = specialNameOf<T>();

			static uint64 construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byName(ART_NAME).front());
			}
		};

		template<Makai::Type::Real T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = specialNameOf<T>();

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byName(ART_NAME).front());
			}
		};

		template<> struct ARTTI<Binary<>> {
			constexpr static scstring ART_NAME = "bytes";

			static Binary<> construct(Object const& value) {
				return value.toValue<Binary<>>();
			}

			static Object::Storage convert(Database<Definition>& db, Binary<> const& value) {
				return Object::create(value, db.byName(ART_NAME).front());
			}
		};

		template<Makai::Type::OneOf<String, UTF8String, UTF32String> T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = "string";

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byName(ART_NAME).front());
			}
		};

		template<Makai::Type::OneOf<Vector2, Vector3, Vector4> T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = "vector";

			static T construct(Object const& value) {
				return value.toValue<Vector4>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(Vector4(value), db.byName(ART_NAME).front());
			}
		};

		template<> struct ARTTI<Matrix4x4> {
			constexpr static scstring ART_NAME = "matrix";

			static Matrix4x4 construct(Object const& value) {
				return value.toValue<Matrix4x4>();
			}

			static Object::Storage convert(Database<Definition>& db, Matrix4x4 const& value) {
				return Object::create(value, db.byName(ART_NAME).front());
			}
		};

		template<ARTType T>
		struct ARTTI<T>: T {
		};

		template <class... Types>
		struct ToObjectTuple {
			using Type = Tuple<Makai::Meta::If<false, Types, Object::Storage>...>;
		};

		template <class... Types>
		struct ListToTuple {
			using Type = ToObjectTuple<Types...>;
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
	constexpr String artnameof() {
		return ARTInfo<T>::ART_NAME;
	}

	template <class... Types>
	constexpr bool fits(List<Object> const& args) {
		return typename Impl::ListToTuple<Types...>::fits();
	}

	template <class... Types>
	constexpr Tuple<Types...> toArguments(List<Object> const& args) {
		return typename Impl::ObjectTupleToArguments<Types...>::make(
			typename Impl::ListToTuple<Types...>::make()
		);
	}
}

#endif
