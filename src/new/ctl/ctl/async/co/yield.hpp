#ifndef CTL_ASYNC_CO_YIELD_H
#define CTL_ASYNC_CO_YIELD_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "promise.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Generates a coroutine that stalls for a given count of yields.
	/// @param count Yield count.
	/// @return Awaitable promise.
	Promise<bool> yield(usize count) {
		while (count--)
			co_yield true;
		co_yield false;
	}
}

CTL_NAMESPACE_END

#endif