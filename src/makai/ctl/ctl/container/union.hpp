#ifndef CTL_CONTAINER_UNION_H
#define CTL_CONTAINER_UNION_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "nullable.hpp"
#include "../meta/pack.hpp"
#include "../typetraits/decay.hpp"
#include "../memory/core.hpp"

CTL_NAMESPACE_BEGIN

namespace Impl {
	template <usize N, class... Types>
	struct SumType;

	template <usize N, class T>
	struct [[gnu::packed, gnu::aligned(1)]] SumType<N, T> {
		constexpr static usize INDEX = N;

		template <usize I>
		constexpr Nullable<T&> get() {
			if constexpr (N != 0) return null;
			else return internal.value;
		}

		template <usize I>
		constexpr Nullable<T> get() const {
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

	template <usize N, class T, class... Types>
	struct [[gnu::packed, gnu::aligned(1)]] SumType<N, T, Types...> {
		constexpr static usize INDEX = N;

		template <usize I>
		constexpr Nullable<Meta::Select<I, T, Types...>&> get() {
			if constexpr (N == 0) return internal.value;
			else return internal.rest.template get<I-1>();
		}

		template <usize I>
		constexpr Nullable<Meta::Select<I, T, Types...>> get() const {
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

	template <class... Types>
	struct [[gnu::packed, gnu::aligned(1)]] Union {
		enum class Value;

		struct IDestructor {
			constexpr virtual ~IDestructor() {}
			constexpr virtual owner<IDestructor> clone() const = 0;
			constexpr virtual owner<IDestructor> newWithAddress(pointer const ptr) const = 0;
		};

		template <class T>
		struct Destructor {
			ref<T> value;
			constexpr virtual ~Destructor() {MX::destruct(value);}

			constexpr virtual owner<IDestructor> clone() const override {
				return new Destructor{value};
			}

			constexpr virtual owner<IDestructor> newWithAddress(pointer const ptr) const override {
				return new Destructor{ptr};
			}
		};

		template <class T>
		consteval static Nullable<usize> indexof() {
			auto const id = Meta::find<T, Types...>();
			if (id == -1) return null;
			return id;
		}

		template <Type::OneOf<Types...> T>
		constexpr Nullable<T&> get() {
			if (tid != indexof<T>())
				return null;
			return sum.template get<indexof<T>().value()>();
		}

		template <Type::OneOf<Types...> T>
		constexpr Nullable<T> get() const {
			if (tid != indexof<T>())
				return null;
			return sum.template get<indexof<T>().value()>();
		}

		template <Type::OneOf<Types...> T>
		constexpr operator Nullable<T&>() {
			return get<T>();
		}

		template <Type::OneOf<Types...> T>
		constexpr operator Nullable<T>() const {
			return get<T>();
		}

		template <Type::OneOf<Types...> T>
		constexpr Union& set(Decay::Unwrap<T> value) {
			constexpr auto id = indexof<T>().value();
			if (destructor)
				delete destructor;
			sum.template set<id>(value);
			auto& newValue = sum.template get<id>();
			destructor = new Destructor<T>(&newValue);
			return *this;
		}

		constexpr ~Union() {
			if (destructor)
				delete destructor;
		}

		constexpr Union(Union const& other): tid(other.tid), sum(other.sum), destructor(tid ? other.destructor->newWithAddress(anull(sum)) : nullptr) {
		}

		constexpr Union(Union&& other): tid(move(other.tid)), sum(move(other.sum)), destructor(move(other.destructor)) {
			other.destructor = nullptr;
		}

		template <Type::OneOf<Types...> T>
		constexpr Union(Decay::Unwrap<T> value) {
			set<T>(value);
		}

	private:
		ref<IDestructor>		destructor = nullptr;
		Nullable<usize>			tid;
		SumType<0, Types...>	sum;
	};
}

template <class... Types>
struct Union: Impl::Union<Types...> {

};

CTL_NAMESPACE_END

#endif
