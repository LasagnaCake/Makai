#ifndef CTL_ADAPTOR_COMPARATOR_H
#define CTL_ADAPTOR_COMPARATOR_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../templates.hpp"
#include "../order.hpp"

CTL_NAMESPACE_BEGIN

template <class A, class B>
struct Comparator;

/// @brief Non-standard threeway comparison detection.
namespace Type::Comparable::NonStandard {
	/// @brief Type must not support threeway comparison, but must support `==`, `>` and `<`.
	template<class A, class B>
	concept FullHouseThreeway =
		!Type::Comparable::Threeway<A, B>
	&&	Type::Comparable::Greater<A, B>
	&&	Type::Comparable::Equals<A, B>
	&&	Type::Comparable::Lesser<A, B>
	;

	/// @brief Type must not support threeway comparison and `==`, but must support `>` and `<`.
	template<class A, class B>
	concept GreaterLesserThreeway =
		!Type::Comparable::Threeway<A, B>
	&&	Type::Comparable::Greater<A, B>
	&&	!Type::Comparable::Equals<A, B>
	&&	Type::Comparable::Lesser<A, B>
	;

	/// @brief Type must not support threeway comparison and `<`, but must support `>` and `==`.
	template<class A, class B>
	concept GreaterEqualsThreeway =
		!Type::Comparable::Threeway<A, B>
	&&	Type::Comparable::Greater<A, B>
	&&	Type::Comparable::Equals<A, B>
	&&	!Type::Comparable::Lesser<A, B>
	;

	/// @brief Type must not support threeway comparison and `>`, but must support `==` and `<`.
	template<class A, class B>
	concept LesserEqualsThreeway =
		!Type::Comparable::Threeway<A, B>
	&&	!Type::Comparable::Greater<A, B>
	&&	Type::Comparable::Equals<A, B>
	&&	Type::Comparable::Lesser<A, B>
	;
	
	/// @brief Type must not support threeway comparison, but instead, must support any non-standard threeway comparison.
	template<class A, class B>
	concept Threeway =
		FullHouseThreeway<A, B>
	||	GreaterLesserThreeway<A, B>
	||	LesserEqualsThreeway<A, B>
	||	GreaterEqualsThreeway<A, B>
	;
}

namespace Type::Comparable {
	/// @brief Typse must be threeway-comparable in some form.
	template<class A, class B>
	concept AnyThreeway =
		Type::Comparable::Threeway<A, B>
	||	Type::Comparable::NonStandard::Threeway<A, B>
	;
}

/// @brief Type comparator adaptor.
/// @tparam A Type to compare.
/// @tparam B Type to compare against.
/// @note This class allows for types without full threeway comparison to behave as if they had.
template <class A, class B>
requires Type::Comparable::AnyThreeway<A, B>
struct Comparator<A, B>: Ordered {
	using typename Ordered::OrderType;

	/// @brief Perfoms a threeway comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static OrderType compare(A const& a, B const& b)
	requires (Type::Comparable::Threeway<A, B>) {
		return OrderType(a <=> b);
	}

	/// @brief Perfoms a threeway comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static OrderType compare(A const& a, B const& b)
	requires Type::Comparable::NonStandard::FullHouseThreeway<A, B> {
		if (a < b)	return Order::LESS;
		if (a == b)	return Order::EQUAL;
		if (a > b)	return Order::GREATER;
		return Order::UNORDERED;
	}

	/// @brief Perfoms a threeway comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static OrderType compare(A const& a, B const& b)
	requires Type::Comparable::NonStandard::GreaterLesserThreeway<A, B> {
		if (a < b)	return Order::LESS;
		if (a > b)	return Order::GREATER;
		return Order::EQUAL;
	}

	/// @brief Perfoms a threeway comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static OrderType compare(A const& a, B const& b)
	requires Type::Comparable::NonStandard::LesserEqualsThreeway<A, B> {
		if (a < b)	return Order::LESS;
		if (a == b)	return Order::EQUAL;
		return Order::GREATER;
	}

	/// @brief Perfoms a threeway comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static OrderType compare(A const& a, B const& b)
	requires Type::Comparable::NonStandard::GreaterEqualsThreeway<A, B> {
		if (a > b)	return Order::GREATER;
		if (a == b)	return Order::EQUAL;
		return Order::LESS;
	}

	/// @brief Perfoms an equality comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static bool equals(A const& a, B const& b)
	requires Type::Comparable::Equals<A, B> {
		return a == b;
	}

	/// @brief Perfoms an equality comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static bool equals(A const& a, B const& b)
	requires (!Type::Comparable::Equals<A, B> && Type::Comparable::Threeway<A, B>) {
		return compare(a, b) == Order::EQUAL;
	}

	/// @brief Perfoms an "greater than" comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static bool greater(A const& a, B const& b) {
		return compare(a, b) == StandardOrder::GREATER;
	}

	/// @brief Perfoms an "lesser than" comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static bool lesser(A const& a, B const& b) {
		return compare(a, b) == StandardOrder::LESS;
	}

	/// @brief Perfoms an "greater than or equal to" comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static bool greaterEquals(A const& a, B const& b) {
		return compare(a, b) == StandardOrder::GREATER || equals(a, b);
	}

	/// @brief Perfoms an "lessser than or equal to" comparison between two values.
	/// @param a Value to compare.
	/// @param b Value to compare against.
	/// @return Result of the comparison.
	constexpr static bool lesserEquals(A const& a, B const& b) {
		return compare(a, b) == StandardOrder::LESS || equals(a, b);
	}
};

/// @brief Comparator-specific type constraints.
namespace Type::Comparator {
	/// @brief Types must be able to be threeway comparable via the compararator.
	template<class A, class B>
	concept Threeway = requires (A a, B b) {
		{::CTL::Comparator<A, B>::compare(a, b)} -> Type::Equal<ValueOrder>;
	};

	/// @brief Types must be able to be equal comparable via the compararator.
	template<class A, class B>
	concept Equals = requires (A a, B b) {
		{::CTL::Comparator<A, B>::equals(a, b)} -> Type::Equal<bool>;
	};

	/// @brief Types must be able to be fully comparable via the compararator.
	template<class A, class B>
	concept Comparable = Threeway<A, B> && Equals<A, B>;
}

/// @brief `Comparator` analog for compaisons between one type.
/// @tparam T Comparable type.
template<class T>
using SimpleComparator = Comparator<T, T>;

CTL_NAMESPACE_END

#endif