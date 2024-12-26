#ifndef CTL_CONTAINER_TUPLE_H
#define CTL_CONTAINER_TUPLE_H

#include "../namespace.hpp"
#include "../templates.hpp"

CTL_NAMESPACE_BEGIN

// Based off off: https://stackoverflow.com/a/52208842

/// @brief General implementations.
namespace Impl {
	/// @brief An element in the tuple.
	/// @tparam T Element type.
	/// @tparam N Element index.
	template<usize N, class T>
	struct TupleItem: Typed<T> {
		using Typed = ::CTL::Typed<T>;

		using typename Typed::DataType;

		DataType value;
	};

	/// @brief Tuple implementation.
	/// @tparam ...Types Tuple types.
	/// @tparam N Tuple index.
	template<usize N, class... Types>
	struct TuplePack;

	/// @brief Empty tuple implementation.
	/// @tparam N Tuple index.
	template<usize N>
	struct TuplePack<N>{};

	/// @brief Tuple implementation.
	/// @tparam T First type.
	/// @tparam ...Types Subsequent types.
	/// @tparam N Tuple index.
	template<usize N, class T, class... Types>
	struct TuplePack<N, T, Types...>:
		public TupleItem<N, T>,
		public TuplePack<N + 1, Types...>,
		Polyglot<T, Types...>
	{
		using Polyglot = Polyglot<T, Types...>;

		using typename Polyglot::DataTypes;

		/// @brief Gets the Nth element in the tuple.
		/// @tparam INDEX Element index.
		/// @return Reference to element.
		template<usize INDEX>
		constexpr auto& get()
		requires (INDEX < DataTypes::COUNT) {
			return get<INDEX>(*this);
		}

		/// @brief Gets the Nth element in a given tuple.
		/// @tparam T1 First tuple type.
		/// @tparam ...Types1 Subsequent tuple types.
		/// @tparam N1 Element index.
		/// @param tup Tuple to index into.
		/// @return Reference to element.
		template<usize N1, class T1, class... Types1>
		constexpr static T1& get(TuplePack<N1, T1, Types1...>& tup)
		requires (N1 < TuplePack<N1, T1, Types1...>::DataTypes::COUNT) {
			return tup.TupleItem<N1, T1>::value;
		}
	};
	
	/// @brief Gets the Nth element in a given tuple.
	/// @tparam T First tuple type.
	/// @tparam ...Types Subsequent tuple types.
	/// @tparam N Element index.
	/// @param tup Tuple to index into.
	/// @return Reference to element.
	template<usize N, class T, class... Types>
	T& get(TuplePack<N, T, Types...>& tup)
	requires (N < TuplePack<N, T, Types...>::DataTypes::COUNT) {
		return tup.TupleItem<N, T>::value;
	}
}

/// @brief Collection of values.
/// @tparam ...Types Element types.
template<class... Types>
using Tuple = Impl::TuplePack<0, Types...>;

CTL_NAMESPACE_END

//#define MAKE_REFLECTIVE(__VA_ARGS__) MemberListType members = {__VA_ARGS__}

#endif // CTL_CONTAINER_TUPLE_H
