#ifndef CTL_RANGE_REVERSE_H
#define CTL_RANGE_REVERSE_H

#include "iterate.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Range-related facilites.
namespace Range {
	/// @brief Reads a range in reverse order. Does not copy the range.
	/// @tparam T Range type.
	/// @param range Range to iterate in reverse order.
	/// @return Reverse iteratable to range.
	template<class T>
	constexpr auto reverse(T& range)
	requires (
		requires (T t) {
			{t.rbegin()};
			{t.rend()};
		}
	) {
		return iterate<decltype(range.rbegin())>(range.rbegin(), range.rend());
	}	
}

CTL_NAMESPACE_END

#endif