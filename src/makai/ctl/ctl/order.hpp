#ifndef CTL_TYPE_ORDER_H
#define CTL_TYPE_ORDER_H

#include <compare>

#include "namespace.hpp"
#include "typeinfo.hpp"
#include "templates.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Element order.
enum class StandardOrder: int {
	LESS = -1,
	EQUAL,
	GREATER,
	UNORDERED
};

/// @brief Compatibility classes between C++'s Standard Template Library and CTL.
namespace Compat {
	/// @brief Wrapper for C++'s `std::partial_ordering`.
	struct Order {
		constexpr static auto LESS		= std::partial_ordering::less;
		constexpr static auto EQUAL		= std::partial_ordering::equivalent;
		constexpr static auto GREATER	= std::partial_ordering::greater;
		constexpr static auto UNORDERED	= std::partial_ordering::unordered;
	};
}

/// @brief Result of a threeway comparison.
struct ValueOrder {
	/// @brief Constructs a `ValueOrder` from a given `StandardOrder`. 
	/// @param value Order.
	constexpr ValueOrder(StandardOrder const& value): val(value) {}

	/// @brief Constructs a `ValueOrder` from a `std::partial_ordering`.
	/// @param value Order.
	constexpr ValueOrder(std::partial_ordering const& value) {
		if (value == Compat::Order::LESS)			val = StandardOrder::LESS;
		else if (value == Compat::Order::GREATER)	val = StandardOrder::GREATER;
		else if (value == Compat::Order::EQUAL)		val = StandardOrder::EQUAL;
	}

	/// @brief Constructs a `ValueOrder` from a `std::weak_ordering`.
	/// @param value Order.
	constexpr ValueOrder(std::weak_ordering const& value):
	ValueOrder(std::partial_ordering(value)) {}

	/// @brief Constructs a `ValueOrder` from a `std::strong_ordering`.
	/// @param value Order.
	constexpr ValueOrder(std::strong_ordering const& value):
	ValueOrder(std::partial_ordering(value)) {}

	/// @brief Threeway comparison operator (`ValueOrder`).
	/// @param other `ValueOrder` to compare with.
	/// @return Order between orders.
	constexpr StandardOrder operator<=>(ValueOrder const& other) const {
		return  ValueOrder(val <=> other.val);
	}

	/// @brief Threeway comparison operator (order).
	/// @param order Order to compare with.
	/// @return Order between order values.
	constexpr StandardOrder operator<=>(StandardOrder const& order) const {
		return  ValueOrder(val <=> order);
	}
	
	/// @brief Threeway comparison operator (value).
	/// @tparam T Value type.
	/// @param value Value to compare with.
	/// @return Order between orders.
	template<Type::Convertible<StandardOrder> T>
	constexpr StandardOrder operator<=>(T const& value)
	requires Type::Different<T, StandardOrder> {
		return  *this <=> (StandardOrder)value;
	}

	/// @brief Threeway comparison operator (value).
	/// @tparam T Value type.
	/// @param value Value to compare with.
	/// @return Order between orders.
	template<Type::Convertible<ValueOrder> T>
	constexpr StandardOrder operator<=>(T const& value)
	requires Type::Different<T, ValueOrder> {
		return  *this <=> ValueOrder(value).order();
	}

	/// @brief Equality comparison operator (order).
	/// @param order Order to compare with.
	/// @return Whether the orders are equal.
	constexpr bool operator==(StandardOrder const& order) const	{return val == order;		}
	/// @brief Equality comparison operator (`ValueOrder`).
	/// @param other `ValueOrder` to compare with.
	/// @return Whether the order values are equal.
	constexpr bool operator==(ValueOrder const& other) const	{return val == other.val;	}
	
	/// @brief Returns whether the order is EQUAL.
	[[nodiscard]]
	friend constexpr bool operator==(ValueOrder const& a, int)	{return a.val == StandardOrder::EQUAL;		}
	/// @brief Returns whether the order is EQUAL.
	[[nodiscard]]
	friend constexpr bool operator==(int, ValueOrder const& b)	{return b == int();							}

	/// @brief Returns whether the order is LESS.
	[[nodiscard]]
	friend constexpr bool operator<(ValueOrder const& a, int)	{return a.val == StandardOrder::LESS;		}
	/// @brief Returns whether the order is GREATER.
	[[nodiscard]]
	friend constexpr bool operator>(ValueOrder const& a, int)	{return a.val == StandardOrder::GREATER;	}
	/// @brief Returns whether the order is LESS or EQUAL.
	[[nodiscard]]
	friend constexpr bool operator<=(ValueOrder const& a, int)	{return a < int() || a == int();			}
	/// @brief Returns whether the order is GREATER or EQUAL.
	[[nodiscard]]
	friend constexpr bool operator>=(ValueOrder const& a, int)	{return a > int() || a == int();			}

	/// @brief Returns whether the order is LESS.
	[[nodiscard]]
	friend constexpr bool operator<(int, ValueOrder const& b)	{return b < int();	}
	/// @brief Returns whether the order is GREATER.
	[[nodiscard]]
	friend constexpr bool operator>(int, ValueOrder const& b)	{return b > int();	}
	/// @brief Returns whether the order is LESS or EQUAL.
	[[nodiscard]]
	friend constexpr bool operator<=(int, ValueOrder const& b)	{return b <= int();	}
	/// @brief Returns whether the order is GREATER or EQUAL.
	[[nodiscard]]
	friend constexpr bool operator>=(int, ValueOrder const& b)	{return b >= int();	}
	
	/// @brief `StandardOrder` type conversion.
	constexpr operator StandardOrder()	{return order();		}
	/// @brief Returns the order value.
	/// @return Order value.
	constexpr StandardOrder order()		{return val;			}

private:
	/// @brief Order value.
	StandardOrder val = StandardOrder::UNORDERED;
};

/// @brief Default threeway comparison operator.
/// @tparam T Left-hand side type.
/// @tparam T2 Right-hand side type.
/// @param a Left-hand side of the operation.
/// @param b Right-hand side of the operation.
/// @return Resulting order.
template<typename T, typename T2>
constexpr ValueOrder operator<=>(T const& a, T const& b) {
	return a <=> b;
}

/// @brief the deriving class as being orderable.
struct Ordered {
	/// @brief Order type.
	typedef ValueOrder OrderType;
	/// @brief Order value.
	using Order = StandardOrder;
};

CTL_NAMESPACE_END

#endif // CTL_TYPE_ORDER_H
