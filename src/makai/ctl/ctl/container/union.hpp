#ifndef CTL_CONTAINER_UNION_H
#define CTL_CONTAINER_UNION_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "../meta/pack.hpp"
#include "../typetraits/decay.hpp"
#include "../typetraits/cast.hpp"
#include "../memory/core.hpp"
#include "nullable.hpp"

CTL_NAMESPACE_BEGIN

namespace Type::Container::Union {
	template <class... Types>
	concept Unitable = (... && (
		Type::Equal<Types, AsNormal<Types>>
	));
}

namespace Impl {
	template <usize N, class... Types>
	struct SumType;

	template <usize N, Type::Container::Union::Unitable T>
	struct [[gnu::packed, gnu::aligned(1)]] SumType<N, T> {
		constexpr static usize INDEX = N;

		template <usize I>
		constexpr Nullable<T&> get() {
			if constexpr (N != 0) return null;
			else return internal.value;
		}

		template <usize I>
		constexpr Nullable<T const&> get() const {
			if constexpr (N != 0) return null;
			else return internal.value;
		}

		template <usize I, class Tx>
		constexpr bool set(Decay::Unwrap<Tx> value) {
			if constexpr (N != 0 or Type::Different<Tx, T>) return false;
			else {
				 internal.value = value;
					return true;
			}
		}

		union Contents {
			T value;
			constexpr ~Contents() {}
		} internal;
	};

	template <usize N, Type::Container::Union::Unitable T, class... Types>
	struct [[gnu::packed, gnu::aligned(1)]] SumType<N, T, Types...> {
		constexpr static usize INDEX = N;

		template <usize I>
		constexpr Nullable<Meta::Select<I, T, Types...>&> get() {
			if constexpr (N == 0) return internal.value;
			else return internal.rest.template get<I-1>();
		}

		template <usize I>
		constexpr Nullable<Meta::Select<I, T, Types...> const&> get() const {
			if constexpr (N == 0) return internal.value;
			else return internal.rest.template get<I-1>();
		}

		template <usize I, class Tx>
		constexpr bool set(Decay::Unwrap<Tx> value) {
			if constexpr (N == 0) {
				if constexpr (Type::Different<Tx, Meta::Select<I, T, Types...>>)
					return false;
				else {
					internal.value = value;
					return true;
				}
			} else return internal.rest.template set<I-1>(value);
		}

		union Contents {
			T value;
			SumType<N+1, Types...> rest;
			constexpr ~Contents() {}
		} internal;
	};


	template <class First, class... Types>
	struct [[gnu::packed, gnu::aligned(1)]] Union {
		static_assert(sizeof...(Types) > 1, "Union types must have two or more type!");
		static_assert(
			Type::Container::Union::Unitable<First, Types...>
			&&	(
				...&& (
					Type::NonVoid<Types>
				&&	Type::NoneOf<Types, Empty>
				)
			), "One or more types are not union-safe!"
		);

		constexpr static bool const CAN_BE_EMPTY = Type::OneOf<First, Empty, void>;

		using BaseType = Meta::If<CAN_BE_EMPTY, SumType<0, Types...>, SumType<0, First, Types...>>;

		template <usize N>
		using Select = Meta::If<CAN_BE_EMPTY, Meta::Select<N, Types...>, Meta::Select<N, First, Types...>>;

		enum class ValueType: ssize {EMPTY = -1};

		template <Type::OneOf<Types...> T>
		consteval static ssize indexof() {
			if constexpr (CAN_BE_EMPTY)
				return Meta::find<T, Types...>();
			else return Meta::find<T, First, Types...>();
		}

		template <Type::OneOf<Types...> T>
		consteval static ValueType match() {
			return ValueType{indexof<T>()};
		}

		template <usize N>
		constexpr auto get() {
			if (tid != N)
				return null;
			return sum.template get<N>();
		}

		template <usize N>
		constexpr auto get() const {
			if (tid != N)
				return null;
			return sum.template get<N>();
		}

		template <Type::OneOf<Types...> T>
		constexpr auto get() {
			if (!tid) return null;
			return get<indexof<T>()>();
		}

		template <Type::OneOf<Types...> T>
		constexpr auto get() const {
			if (!tid) return null;
			return get<indexof<T>()>();
		}

		template <Type::OneOf<Types...> T>
		constexpr operator Nullable<T&>() {
			return get<T>();
		}

		template <Type::OneOf<Types...> T>
		constexpr operator Nullable<T const&>() const {
			return get<T>();
		}

		template <Type::OneOf<Types...> T>
		constexpr Union& set(Decay::Unwrap<T> value)
		requires (Type::NoneOf<T, Empty, void>) {
			constexpr auto id = indexof<T>();
			unset();
			sum.template set<id>(value);
			tid = id;
			return *this;
		}

		template <Type::Equal<Empty> T>
		constexpr Union& set(Decay::Unwrap<T> value)
	 	requires (CAN_BE_EMPTY) {
			return unset();
		}

		constexpr Union& clear() requires (CAN_BE_EMPTY) {
			return unset();
		}

		constexpr ~Union() {unset();}

		constexpr Union()		requires (CAN_BE_EMPTY) {}
		constexpr Union(Empty)	requires (CAN_BE_EMPTY) {}

		constexpr Union(Union const& other): tid(other.tid), sum(other.sum) {
		}

		constexpr Union(Union&& other): tid(move(other.tid)), sum(move(other.sum)) {
		}

		template <Type::OneOf<Types...> T>
		constexpr Union(Decay::Unwrap<T> value)
		requires Type::NoneOf<T, Empty, void> {
			set<T>(value);
		}

		template <class... TVisits>
		constexpr bool visit(TVisits const&... visits) {
			return (... or visitFor(visits));
		}

		template <class... TVisits>
		constexpr bool visit(TVisits const&... visits) const {
			return (... or visitFor(visits));
		}

		constexpr ValueType type() const {
			if (!tid) return ValueType::EMPTY;
			return Cast::as<ValueType>(*tid);
		}

		template <class T>
		constexpr bool is() const {
			if (!tid && indexof<T>() == -1) return true;
			return tid == indexof<T>();
		}

		constexpr bool is(ValueType const t) const {
			if (!tid && t == ValueType::EMPTY) return true;
			return tid == Cast::as<usize>(t);
		}

	private:
		constexpr Union& unset() {
			if (tid)
				destruct<0>();
			tid = null;
			return *this;
		}

		template <usize N>
		constexpr void destruct() {
			using Tx = Select<N>;
			if (N == tid) MX::destruct<Tx>(&get<Tx>());
			else destruct<N+1>();
		}

		template <Type::NoneOf<void, Empty> T, Type::Functional<void(T&)> TVisit>
		constexpr bool visitFor(TVisit const& fn) {
			if (tid != indexof<T>()) return false;
			fn(get<T>().value());
			return true;
		}

		template <Type::NoneOf<void, Empty> T, Type::Functional<void(T const&)> TVisit>
		constexpr bool visitFor(TVisit const& fn) const {
			if (tid != indexof<T>()) return false;
			fn(get<T>().value());
			return true;
		}

		Nullable<usize>	tid;
		BaseType		sum;
	};
}

template <class... Types>
using Union = Impl::Union<Types...>;

template <class... Types>
using Join = Meta::Any<
	Meta::When<sizeof...(Types) == 0, Meta::Invalid>,
	Meta::When<sizeof...(Types) == 1, Meta::First<Types...>>,
	Union<Types...>
>;

CTL_NAMESPACE_END

#endif
