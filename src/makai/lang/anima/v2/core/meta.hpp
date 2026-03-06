#ifndef MAKAILIB_ANIMA_V2_CORE_META_H
#define MAKAILIB_ANIMA_V2_CORE_META_H

#include "type.hpp"
#include "method.hpp"
#include "value.hpp"

namespace Makai::Anima::V2::Core::Meta {
	struct Void	{};
	struct Any	{};

	namespace Impl {
		template <class T> struct ARTTI;

		template <class T>
		concept ARTType = requires {
			{T::ART_NAME}		-> Type::Equal<scstring>;
			{T::constructor()}	-> Type::Functional<T(Object)>;
			{T::converter()}	-> Type::Functional<Object(Definition::Database&, Makai::Meta::If<Type::Void<T>, nulltype, T> const&)>;
		};

		template <class T>
		constexpr Function<T(Object const&)> toValue() {
			return [] (Object const& obj) -> T {return obj.value.as<T>();};
		}

		template <class T>
		constexpr Function<T(Object const&)> toEmpty() {
			return [] (Object const& obj) -> T {return T{};};
		}

		template<> struct ARTTI<Void> {
			constexpr static scstring ART_NAME = "void";

			constexpr static Function<Void(Object const&)> constructor() {
				return toEmpty<Void>();
			}

			constexpr static Function<Object(Definition::Database&, nulltype&)> converter() {
				return [] (Definition::Database&, nulltype&) -> Object {
					return Object();
				};
			}
		};

		template<> struct ARTTI<Any> {
			constexpr static scstring ART_NAME = "any";

			constexpr static Function<Any(Object const&)> constructor() {
				return toEmpty<Any>();
			}
		};

		template<> struct ARTTI<nulltype> {
			constexpr static scstring ART_NAME = "nil";

			constexpr static Function<nulltype(Object const&)> constructor() {
				return toEmpty<nulltype>();
			}
		};

		template<> struct ARTTI<bool> {
			constexpr static scstring ART_NAME = "bool";
			constexpr static auto constructor() {
				return toValue<bool>();
			}
		};

		template<Type::SignedInteger T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = "int";
			constexpr static auto constructor() {
				return toValue<int64>();
			}
		};

		template<Type::UnsignedInteger T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = "uint";
			constexpr static auto constructor() {
				return toValue<uint64>();
			}
		};

		template<Type::Real T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = "real";
			constexpr static auto constructor() {
				return toValue<double>();
			}
		};

		template<Type::Equal<Binary<>> T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = "bytes";
			constexpr static auto constructor() {
				return toValue<Binary<>>();
			}
		};

		template<Type::OneOf<String, UTF8String> T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = "string";
			constexpr static auto constructor() {
				return toValue<UTF8String>();
			}
		};

		template<Type::OneOf<Vector2, Vector3, Vector4> T> struct ARTTI<T> {
			constexpr static scstring ART_NAME = "vector";
			constexpr static auto constructor() {
				return toValue<Vector4>();
			}
		};

		template<ARTType T>
		struct ARTTI<T>: T {
		};

		template <class... Types>
		struct ToObjectTuple {
			using Type = Tuple<Makai::Meta::If<false, Types, Object>...>;
		};

		template <class... Types>
		struct ListToTuple {
			using Type = ToObjectTuple<Types...>;
			using ListType = List<Object>;

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
				return {ARTTI<Makai::Meta::Select<N, Types...>>::converter()(tup.template get<N>())...};
			}
		};
	};

	template <class T>
	constexpr String nameof() {
		return Impl::ARTTI<T>::ART_NAME;
	}

	template <class T>
	constexpr auto constructor() {
		return Impl::ARTTI<T>::constructor();
	}

	template <class T>
	constexpr auto converter() {
		return Impl::ARTTI<T>::converter();
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
