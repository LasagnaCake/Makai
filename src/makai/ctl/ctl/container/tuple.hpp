#ifndef CTL_CONTAINER_TUPLE_H
#define CTL_CONTAINER_TUPLE_H

#include "../namespace.hpp"
#include "../templates.hpp"

CTL_NAMESPACE_BEGIN

// Based off off: https://stackoverflow.com/a/52208842

/// @brief General implementations.
namespace Impl {
	/// @brief Tuple implementation.
	/// @tparam ...Types Tuple types.
	template<class... Types>
	struct Tuple;

	/// @brief Tuple implementation.
	/// @tparam T Element type.
	template<class T>
	struct Tuple<T>:
		Typed<T> {
		using Typed = Typed<T>;

		using typename Typed::DataType;

		constexpr static bool NON_CONST_TUPLE = Type::NonConstant<T>;
		
		/// @brief Nth element type.
		/// @tparam I Type index.
		template<usize I>
		using TupleType = DataType;

		/// @brief Default constructor (defaulted).
		constexpr Tuple()				= default;
		/// @brief Copy constructor (defaulted).
		constexpr Tuple(Tuple&&)		= default;
		/// @brief Move constructor (defaulted).
		constexpr Tuple(Tuple const&)	= default;

		/// @brief Constructs the tuple.
		/// @param first Element value.
		constexpr Tuple(DataType const& first):							value(first)		{}
		/// @brief Constructs the tuple.
		/// @param first Element value.
		constexpr Tuple(DataType&& first) requires (NON_CONST_TUPLE):	value(move(first))	{}

		/// @brief Gets the Nth element in the tuple.
		/// @tparam INDEX Element index.
		/// @return Reference to element.
		template<usize I>
		constexpr DataType& get() 
		requires (I == 0) {
			return value;
		}

		/// @brief Gets the Nth element in the tuple.
		/// @tparam INDEX Element index.
		/// @return Reference to element.
		template<usize I>
		constexpr DataType const& get() const
		requires (I == 0) {
			return value;
		}

	private:
		/// @brief Tuple value.
		DataType value;
	};

	/// @brief Tuple implementation.
	/// @tparam T First type.
	/// @tparam ...Types Subsequent types.
	template<class T, class... Types>
	struct Tuple<T, Types...>:
		Polyglot<T, Types...> {
		using Polyglot = Polyglot<T, Types...>;

		using typename Polyglot::DataTypes;

		using DataType = typename Polyglot::DataTypes::FirstType;

		/// @brief Nth element type.
		/// @tparam I Type index.
		template<usize I>
		using TupleType = typename DataTypes::template Type<I>;

		/// @brief Default constructor (defaulted).
		constexpr Tuple()				= default;
		/// @brief Copy constructor (defaulted).
		constexpr Tuple(Tuple&&)		= default;
		/// @brief Move constructor (defaulted).
		constexpr Tuple(Tuple const&)	= default;

		/// @brief Constructs the tuple.
		/// @param first Frst element value.
		/// @param ...rest Subsequent element values.
		constexpr Tuple(T const& first, Types const&... rest):	value(first), rest(rest...)				{}
		/// @brief Constructs the tuple.
		/// @param first Frst element value.
		/// @param ...rest Subsequent element values.
		constexpr Tuple(T&& first, Types&&... rest):			value(move(first)), rest(move(rest)...)	{}

		/// @brief Gets the Nth element in the tuple.
		/// @tparam INDEX Element index.
		/// @return Reference to element.
		template<usize I>
		constexpr TupleType<I>& get()
		requires (I > 0 && I < DataTypes::COUNT) {
			return rest.template get<I-1>();
		}

		/// @brief Gets the Nth element in the tuple.
		/// @tparam INDEX Element index.
		/// @return Reference to element.
		template<usize I>
		constexpr TupleType<I> const& get() const
		requires (I > 0 && I < DataTypes::COUNT) {
			return rest.template get<I-1>();
		}

		/// @brief Gets the Nth element in the tuple.
		/// @tparam INDEX Element index.
		/// @return Reference to element.
		template<usize I> constexpr TupleType<I>& get() requires (I == 0)				{return value;}

		/// @brief Gets the Nth element in the tuple.
		/// @tparam INDEX Element index.
		/// @return Reference to element.
		template<usize I> constexpr TupleType<I> const& get() const requires (I == 0)	{return value;}

		template <usize... N>
		constexpr Tuple<TupleType<N>...> reduced() const requires (sizeof...(N) > 0) {
			return Tuple<TupleType<N>...>(get<N>()...);
		}

	private:
		/// @brief Tuple value.
		DataType value;
		/// @brief Other values.
		Tuple<Types...> rest;
	};

	/// @brief Index tuple item.
	/// @tparam N Tuple index.
	/// @tparam V Index value.
	template<usize N, usize V>
	struct IndexTupleItem: ValueConstant<usize, V> {};

	/// @brief Index tuple implementation.
	/// @tparam N Tuple index.
	/// @tparam V... Values.
	template<usize N, usize... V>
	struct IndexTuplePack;

	/// @brief Index tuple implementation.
	/// @tparam N Tuple index.
	/// @tparam F First value.
	template<usize N, usize F>
	struct IndexTuplePack<N, F>: IndexTupleItem<N, F> {};

	/// @brief Index tuple implementation.
	/// @tparam N Tuple index.
	/// @tparam F First value.
	/// @tparam R... Subsequent values.
	template<usize N, usize F, usize... R>
	struct IndexTuplePack<N, F, R...>:
		public IndexTupleItem<N, F>,
		public IndexTuplePack<F + 1, R...> {
		/// @brief Gets the Nth element in the tuple.
		/// @tparam INDEX Element index.
		/// @return Element value.
		template<usize INDEX>
		consteval usize get() const {
			return get<INDEX>(*this);
		}

		/// @brief Gets the Nth element in a given tuple.
		/// @tparam F1 First tuple index value.
		/// @tparam ...R1 Subsequent tuple index values.
		/// @tparam N1 Element index.
		/// @param tup Tuple to index into.
		/// @return Element value.
		template<usize N1, usize F1, usize... R1>
		consteval static usize get(IndexTuplePack<N1, F1, R1...>& tup) {
			return tup.IndexTupleItem<N1, F1>::value;
		}
	};
}

/// @brief Collection of values.
/// @tparam ...Types Element types.
template<class... Types>
using Tuple = Impl::Tuple<Types...>;

/// @brief Collection of indices.
/// @tparam ...I Indices.
template<usize... I>
using IndexTuple = Impl::IndexTuplePack<0, I...>;

/// @brief Builtins demangler.
namespace Impl::Builtin {
#if defined(__clang__)
	namespace {	
		template <class T, usize... V>
		using IntegerPackWrapper = IndexTuple<V...>;
	}
	template <usize N>
	using MakePack = __make_integer_seq<IntegerPackWrapper, usize, N>;
#else
	template <usize N>
	using MakePack = IndexTuple<__integer_pack(N)...>;
#endif
}

/// @brief Creates an integer pack from [0 -> N-1].
/// @tparam N Pack size.
template<usize N>
using IntegerPack = Impl::Builtin::MakePack<N>;

/// @brief Gets the type of the Nth element in a tuple.
/// @tparam T Tuple type.
/// @tparam N Element type.
template<class T, usize N>
using TupleType = typename T::template TupleType<N>;

/// @brief Gets the Nth element in a given tuple.
/// @tparam I Element index.
/// @tparam ...Types Tuple types.
/// @param tuple Tuple to index into.
/// @return Reference to element.
template <usize I, class... Types>
constexpr TupleType<Tuple<Types...>, I>& get(Tuple<Types...>& tuple) {
	return tuple.template get<I>();
}

/// @brief Gets the Nth element in a given tuple.
/// @tparam I Element index.
/// @tparam ...Types Tuple types.
/// @param tuple Tuple to index into.
/// @return Const reference to element.
template <usize I, class... Types>
constexpr TupleType<Tuple<Types...>, I> const& get(Tuple<Types...> const& tuple) {
	return tuple.template get<I>();
}

/// @brief Gets the Nth element in a given index tuple.
/// @tparam I Element index.
/// @tparam ...V Tuple values.
/// @return Element value.
template<usize I, usize... V>
consteval usize get(IndexTuple<V...> const& tuple) {
	return tuple.template get<I>();
}

CTL_NAMESPACE_END

//#define MAKE_REFLECTIVE(__VA_ARGS__) MemberListType members = {__VA_ARGS__}

#endif // CTL_CONTAINER_TUPLE_H
