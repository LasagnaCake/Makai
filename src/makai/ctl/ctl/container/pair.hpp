#ifndef CTL_CONTAINER_PAIR_H
#define CTL_CONTAINER_PAIR_H

#include "../templates.hpp"
#include "../adapter/comparator.hpp"
#include "../meta/logic.hpp"

CTL_NAMESPACE_BEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Type must be a valid pair type.
	template<class T>
	concept PairLike = requires (T t1, T const t2) {
		typename T::AType;
		typename T::BType;
		{t1.front()} -> EqualOrConst<typename T::AType&>;
		{t1.back()} -> EqualOrConst<typename T::BType&>;
		{t2.front()} -> EqualOrConst<typename T::AType const&>;
		{t2.back()} -> EqualOrConst<typename T::BType const&>;
		requires Constructible<T, typename T::AType, typename T::BType>;
	};
}

template<typename TA, typename TB> struct Pair;

/// @brief Custom comparator implementation for a pair-type.
template<Type::Container::PairLike TPair>
struct PairComparator: Ordered {
	/// @brief Pair type.
	using PairType = TPair;

	/// @brief "A" type.
	using AType = typename TPair::AType;
	/// @brief "B" type.
	using BType = typename TPair::BType;

	using typename Ordered::OrderType;

	/// @brief Whether "A" type is comparable.
	constexpr static bool COMPARABLE_A = Type::Comparator::Comparable<AType, AType>;
	/// @brief Whether "B" type is comparable.
	constexpr static bool COMPARABLE_B = Type::Comparator::Comparable<BType, BType>;

	/// @brief Whether this comparator is valid.
	constexpr static bool IS_COMPARABLE = COMPARABLE_A || COMPARABLE_B;

	/// @brief Comparator for "A" type (if comparable).
	using AComparator	= Meta::DualType<COMPARABLE_A, Comparator<AType, AType>, void>;
	/// @brief Comparator for "B" type (if comparable).
	using BComparator	= Meta::DualType<COMPARABLE_B, Comparator<BType, BType>, void>;

	/// @brief Compares two pairs.
	/// @param a Left-hand side of the operation.
	/// @param b Right-hand side of the operation.
	/// @return Order between pairs.
	constexpr static OrderType compare(PairType const& a, PairType const& b)
	requires (COMPARABLE_A && COMPARABLE_B) {
		OrderType order = AComparator::compare(a.front(), b.front());
		if (order == Order::EQUAL)
			return BComparator::compare(a.back(), b.back());
		return order;
	}

	/// @brief Compares two pairs.
	/// @param a Left-hand side of the operation.
	/// @param b Right-hand side of the operation.
	/// @return Order between pairs.
	constexpr static OrderType compare(PairType const& a, PairType const& b)
	requires (COMPARABLE_A && !COMPARABLE_B) {
		return AComparator::compare(a.front(), b.front());
	}

	/// @brief Compares two pairs.
	/// @param a Left-hand side of the operation.
	/// @param b Right-hand side of the operation.
	/// @return Order between pairs.
	constexpr static OrderType compare(PairType const& a, PairType const& b)
	requires (!COMPARABLE_A && COMPARABLE_B) {
		return BComparator::compare(a.back(), b.back());
	}
};

/// @brief Tags the deriving class as a pair of values.
/// @tparam TA "A" type.
/// @tparam TB "B" type.
template<typename TA, typename TB>
struct Pairable {
	/// @brief "A" type.
	typedef TA AType;
	/// @brief "B" type.
	typedef TB BType;

	/// @brief Pair decay type.
	typedef Pair<AType, BType> PairType;
};

/// @brief A-B pair.
/// @tparam TA "A" type. 
/// @tparam TB "B" type.
template<typename TA, typename TB>
struct Pair {
	using SelfIdentified	= ::CTL::SelfIdentified<Pair<TA, TB>>;
	using Pairable			= ::CTL::Pairable<TA, TB>;

	using
		SelfType = typename SelfIdentified::SelfType
	;

	using AType		= typename Pairable::AType;
	using BType		= typename Pairable::BType;
	using PairType	= typename Pairable::PairType;

	using OrderType = typename Ordered::OrderType;

	/// @brief "A" value.
	AType a;
	/// @brief "B" value.
	BType b;

	/// @brief Threeway comparison operator.
	/// @param other Other `Pair` to compare with.
	/// @return Order between objects.
	constexpr OrderType operator<=>(SelfType const& other) const
	requires (PairComparator<SelfType>::IS_COMPARABLE) {
		return PairComparator<SelfType>::compare(*this, other);
	}

	/// @brief Returns `a`.
	/// @return Reference to `a`.
	constexpr AType& front()				{return a;	}
	/// @brief Returns `b`.
	/// @return Reference to `b`.
	constexpr BType& back()					{return b;	}
	/// @brief Returns `a`.
	/// @return Const reference to `a`.
	constexpr AType const& front() const	{return a;	}
	/// @brief Returns `a`.
	/// @return Const reference to `a`.
	constexpr BType const& back() const		{return b;	}

	/// @brief Converts the object to another pair-esque type.
	/// @tparam TPair Pair-esque type.
	/// @return Pair-esque object.
	template<Type::Constructible<AType, BType> TPair>
	constexpr TPair pair() const		{return TPair(a, b);	}

	/// @brief Converts the object to another pair-esque type.
	/// @tparam TPair Pair-esque type.
	/// @return Pair-esque object.
	template<Type::Constructible<AType, BType> TPair>
	constexpr operator TPair() const	{return pair<TPair>();	}
};

/// @brief Key-Value pair.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
template<typename TKey, typename TValue>
struct KeyValuePair {
	using SelfIdentified	= ::CTL::SelfIdentified<KeyValuePair<TKey, TValue>>;
	using Pairable			= ::CTL::Pairable<TKey, TValue>;

	using
		SelfType = typename SelfIdentified::SelfType
	;

	using AType		= typename Pairable::AType;
	using BType		= typename Pairable::BType;
	using PairType	= typename Pairable::PairType;

	using OrderType = typename Ordered::OrderType;

	/// @brief Key.
	AType	key;
	/// @brief Value.
	BType	value;
	
	/// @brief Threeway comparison operator.
	/// @param other Other `KeyValuePair` to compare with.
	/// @return Order between objects.
	constexpr OrderType operator<=>(SelfType const& other) const
	requires (PairComparator<SelfType>::IS_COMPARABLE) {
		return PairComparator<SelfType>::compare(*this, other);
	}

	/// @brief Returns `key`.
	/// @return Reference to `key`.
	constexpr AType& front()				{return key;	}
	/// @brief Returns `value`.
	/// @return Reference to `value`.
	constexpr BType& back()					{return value;	}
	/// @brief Returns `key`.
	/// @return Const reference to `key`.
	constexpr AType const& front() const	{return key;	}
	/// @brief Returns `value`.
	/// @return Const reference to `value`.
	constexpr BType const& back() const		{return value;	}

	/// @brief Converts the object to a `Pair`.
	/// @return Object as `Pair`.
	constexpr PairType pair() const		{return PairType(key, value);	}
	/// @brief Converts the object to a `Pair`.
	/// @return Object as `Pair`.
	constexpr operator PairType() const	{return pair();					}
};

/// @brief Left-Right pair.
/// @tparam TLeft Left type.
/// @tparam TRight Right type.
template<typename TLeft, typename TRight>
struct LeftRightPair {
	using SelfIdentified	= ::CTL::SelfIdentified<LeftRightPair<TLeft, TRight>>;
	using Pairable			= ::CTL::Pairable<TLeft, TRight>;

	using
		SelfType = typename SelfIdentified::SelfType
	;

	using AType		= typename Pairable::AType;
	using BType		= typename Pairable::BType;
	using PairType	= typename Pairable::PairType;

	using OrderType = typename Ordered::OrderType;

	/// @brief Left side.
	AType	left;
	/// @brief Right side.
	BType	right;

	/// @brief Threeway comparison operator.
	/// @param other Other `LeftRightPair` to compare with.
	/// @return Order between objects.
	constexpr OrderType operator<=>(SelfType const& other) const
	requires (PairComparator<SelfType>::IS_COMPARABLE) {
		return PairComparator<SelfType>::compare(*this, other);
	}

	/// @brief Returns `left`.
	/// @return Reference to `left`.
	constexpr AType& front()				{return left;	}
	/// @brief Returns `right`.
	/// @return Reference to `right`.
	constexpr BType& back()					{return right;	}
	/// @brief Returns `left`.
	/// @return Const reference to `left`.
	constexpr AType const& front() const	{return left;	}
	/// @brief Returns `right`.
	/// @return Const reference to `right`.
	constexpr BType const& back() const		{return right;	}

	/// @brief Converts the object to a `Pair`.
	/// @return Object as `Pair`.
	constexpr PairType pair() const		{return PairType(left, right);	}
	/// @brief Converts the object to a `Pair`.
	/// @return Object as `Pair`.
	constexpr operator PairType() const	{return pair();					}
};

/// @brief First-Second (STL-like) pair.
/// @tparam T1 First type.
/// @tparam T2 Second type.
template<typename T1, typename T2>
struct FirstSecondPair {
	using SelfIdentified	= ::CTL::SelfIdentified<FirstSecondPair<T1, T2>>;
	using Pairable			= ::CTL::Pairable<T1, T2>;

	using
		SelfType = typename SelfIdentified::SelfType
	;

	using AType		= typename Pairable::AType;
	using BType		= typename Pairable::BType;
	using PairType	= typename Pairable::PairType;

	using OrderType = typename Ordered::OrderType;

	/// @brief First value.
	AType	first;
	/// @brief Second value.
	BType	second;
	
	/// @brief Threeway comparison operator.
	/// @param other Other `FirstSecondPair` to compare with.
	/// @return Order between objects.
	constexpr OrderType operator<=>(SelfType const& other) const
	requires (PairComparator<SelfType>::IS_COMPARABLE) {
		return PairComparator<SelfType>::compare(*this, other);
	}

	/// @brief Returns `first`.
	/// @return Reference to `first`.
	constexpr AType& front()				{return first;	}
	/// @brief Returns `second`.
	/// @return Reference to `second`.
	constexpr BType& back()					{return second;	}
	/// @brief Returns `first`.
	/// @return Const reference to `first`.
	constexpr AType const& front() const	{return first;	}
	/// @brief Returns `second`.
	/// @return Const reference to `second`.
	constexpr BType const& back() const		{return second;	}

	/// @brief Converts the object to a `Pair`.
	/// @return Object as `Pair`.
	constexpr PairType pair() const		{return PairType(first, second);	}
	/// @brief Converts the object to a `Pair`.
	/// @return Object as `Pair`.
	constexpr operator PairType() const	{return pair();						}
};

/// @brief Tags the deriving class containing a pair of some sort.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TPair<class, class> Pair type.
template<
	class TKey,
	class TValue,
	template <class TPairKey, class TPairValue> class TPair = Pair
>
struct Paired {
	static_assert(Type::Container::PairLike<TPair<TKey, TValue>>, "Type is not a valid pair type!");

	/// @brief Key type.
	typedef TKey				KeyType;
	/// @brief Value type.
	typedef TValue				ValueType;
	/// @brief Pair type.
	typedef TPair<TKey, TValue>	PairType;
};

static_assert(Type::Comparator::Threeway<Pair<int, int>, Pair<int, int>>);
static_assert(Type::Comparator::Threeway<KeyValuePair<int, int>, KeyValuePair<int, int>>);
static_assert(Type::Comparator::Threeway<LeftRightPair<int, int>, LeftRightPair<int, int>>);
static_assert(Type::Comparator::Threeway<FirstSecondPair<int, int>, FirstSecondPair<int, int>>);

#pragma GCC diagnostic pop

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_PAIR_H
