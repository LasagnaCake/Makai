#ifndef CTL_ASYNC_CO_YIELD_H
#define CTL_ASYNC_CO_YIELD_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "promise.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Generates a coroutine that stalls a given amount of times.
	/// @param count Yield count.
	/// @return Awaitable promise.
	/// @details
	///		Generates a coroutine that will return `true` for the given amount of times passed.
	///		Coroutine is always starts suspended.
	///
	///		Meant to be used in conjunction with a `while` loop — see example.
	/// @example
	///		auto yielder = yield(/*count*/);
	///		while (yielder.next())
	///			co_yield /*return type*/;
	Promise<bool, false> yield(usize count) {
		if (!count) co_return false;
		while (count--)
			co_yield true;
		co_yield false;
	}
}

CTL_NAMESPACE_END

#endif