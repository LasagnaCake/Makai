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
	concept PairLike = requires (T t) {
		typename T::AType;
		typename T::BType;
		{t.front()} -> EqualOrConst<typename T::AType&>;
		{t.back()} -> EqualOrConst<typename T::BType&>;
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
struct Pair:
	Ordered,
	SelfIdentified<Pair<TA, TB>>,
	Pairable<TA, TB> {
	using SelfIdentified	= ::CTL::SelfIdentified<Pair<TA, TB>>;
	using Pairable			= ::CTL::Pairable<TA, TB>;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Pairable::AType,
		typename Pairable::BType,
		typename Pairable::PairType
	;

	using typename Ordered::OrderType;

	/// @brief "A" value.
	AType a;
	/// @brief "B" value.
	BType b;

	/// @brief Empty constructor.
	constexpr Pair() = default;
	/// @brief Constructs only `a`.
	/// @param a Value of `a`.
	constexpr Pair(AType const& a):					a(a)								{}
	/// @brief Constructs both `a` and `b`. 
	/// @param a Value of `a`.
	/// @param b Value of `b`.
	constexpr Pair(AType const& a, BType const& b):	a(a), b(b)							{}
	/// @brief Copy constructor.
	/// @param other `Pair` to copy from.
	constexpr Pair(SelfType const& other):			a(other.a), b(other.b)				{}
	/// @brief Copy constructor (pair-like).
	/// @tparam T Pair-like type.
	/// @param other Pair-like object to copy from.
	template<Type::Container::PairLike T>
	constexpr Pair(T const& other):					a(other.front()), b(other.back())	{}

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
	constexpr BType& back()				{return b;	}
	/// @brief Returns `a`.
	/// @return Const reference to `a`.
	constexpr AType const& front() const	{return a;	}
	/// @brief Returns `a`.
	/// @return Const reference to `a`.
	constexpr BType const& back() const	{return b;	}

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
struct KeyValuePair:
	Ordered,
	SelfIdentified<KeyValuePair<TKey, TValue>>,
	Pairable<TKey, TValue> {
	using SelfIdentified	= ::CTL::SelfIdentified<KeyValuePair<TKey, TValue>>;
	using Pairable			= ::CTL::Pairable<TKey, TValue>;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Pairable::AType,
		typename Pairable::BType,
		typename Pairable::PairType
	;

	using typename Ordered::OrderType;

	/// @brief Key.
	AType	key;
	/// @brief Value.
	BType	value;

	/// @brief Empty constructor.
	constexpr KeyValuePair() = default;

	/// @brief Constructs only the key.
	/// @param k Value of key.
	constexpr KeyValuePair(AType const& k):					key(k)									{}
	/// @brief Constructs both key and value.
	/// @param k Value of `key`.
	/// @param v Value of `value`.
	constexpr KeyValuePair(AType const& k, BType const& v):	key(k), value(v)						{}
	/// @brief Copy constructor (Pair-like).
	/// @param other Other pair-like object.
	constexpr explicit KeyValuePair(PairType const& other):	KeyValuePair(other.a, other.b)			{}
	/// @brief Copy constructor (`KeyValuePair`).
	/// @param other Other `KeyValuePair`.
	constexpr KeyValuePair(SelfType const& other):			KeyValuePair(other.key, other.value)	{}

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
	constexpr BType& back()				{return value;	}
	/// @brief Returns `key`.
	/// @return Const reference to `key`.
	constexpr AType const& front() const	{return key;	}
	/// @brief Returns `value`.
	/// @return Const reference to `value`.
	constexpr BType const& back() const	{return value;	}

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
struct LeftRightPair:
	Ordered,
	SelfIdentified<LeftRightPair<TLeft, TRight>>,
	Pairable<TLeft, TRight> {
	using SelfIdentified	= ::CTL::SelfIdentified<LeftRightPair<TLeft, TRight>>;
	using Pairable			= ::CTL::Pairable<TLeft, TRight>;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Pairable::AType,
		typename Pairable::BType,
		typename Pairable::PairType
	;

	using typename Ordered::OrderType;

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
	constexpr BType& back()				{return right;	}
	/// @brief Returns `left`.
	/// @return Const reference to `left`.
	constexpr AType const& front() const	{return left;	}
	/// @brief Returns `right`.
	/// @return Const reference to `right`.
	constexpr BType const& back() const	{return right;	}

	/// @brief Empty constructor.
	constexpr LeftRightPair() = default;
	
	/// @brief Constructs only the left side.
	/// @param l Value of the left side.
	constexpr LeftRightPair(AType const& l):					left(l)									{}
	/// @brief Constructs both the left and right sides.
	/// @param l Value of the left side.
	/// @param r Value of the right side.
	constexpr LeftRightPair(AType const& l, BType const& r):	left(l), right(r)						{}
	/// @brief Copy constructor (Pair-like).
	/// @param other Other pair-like object.
	constexpr explicit LeftRightPair(PairType const& other):	LeftRightPair(other.a, other.b)			{}
	/// @brief Copy constructor (`LeftRightPair`).
	/// @param other Other `LeftRightPair`.
	constexpr LeftRightPair(SelfType const& other):				LeftRightPair(other.left, other.right)	{}

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
struct FirstSecondPair:
	Ordered,
	SelfIdentified<FirstSecondPair<T1, T2>>,
	Pairable<T1, T2> {
	using SelfIdentified	= ::CTL::SelfIdentified<FirstSecondPair<T1, T2>>;
	using Pairable			= ::CTL::Pairable<T1, T2>;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Pairable::AType,
		typename Pairable::BType,
		typename Pairable::PairType
	;

	using typename Ordered::OrderType;

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
	constexpr BType& back()				{return second;	}
	/// @brief Returns `first`.
	/// @return Const reference to `first`.
	constexpr AType const& front() const	{return first;	}
	/// @brief Returns `second`.
	/// @return Const reference to `second`.
	constexpr BType const& back() const	{return second;	}

	/// @brief Empty constructor.
	constexpr FirstSecondPair() = default;

	/// @brief Constructs only the first value.
	/// @param v1 First value.
	constexpr FirstSecondPair(AType const& v1):						first(v1)									{}
	/// @brief Constructs both first and second values.
	/// @param v1 First value.
	/// @param v2 Second value.
	constexpr FirstSecondPair(AType const& v1, BType const& v2):	first(v1), second(v2)						{}
	/// @brief Copy constructor (Pair-like).
	/// @param other Other pair-like object.
	constexpr explicit FirstSecondPair(PairType const& other):		FirstSecondPair(other.a, other.b)			{}
	/// @brief Copy constructor (`FirstSecondPair`).
	/// @param other Other `FirstSecondPair`.
	constexpr FirstSecondPair(SelfType const& other):				FirstSecondPair(other.first, other.second)	{}

	/// @brief Converts the object to a `Pair`.
	/// @return Object as `Pair`.
	constexpr PairType pair() const		{return PairType(first, second);	}
	/// @brief Converts the object to a `Pair`.
	/// @return Object as `Pair`.
	constexpr operator PairType() const	{return pair();						}
};

static_assert(Type::Comparator::Threeway<Pair<int, int>, Pair<int, int>>);
static_assert(Type::Comparator::Threeway<KeyValuePair<int, int>, KeyValuePair<int, int>>);
static_assert(Type::Comparator::Threeway<LeftRightPair<int, int>, LeftRightPair<int, int>>);
static_assert(Type::Comparator::Threeway<FirstSecondPair<int, int>, FirstSecondPair<int, int>>);

#pragma GCC diagnostic pop

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_PAIR_H
