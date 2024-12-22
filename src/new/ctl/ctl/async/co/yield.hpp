#ifndef CTL_ASYNC_CO_YIELD_H
#define CTL_ASYNC_CO_YIELD_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "promise.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	Promise<bool> yield(usize count) {
		while (count--)
			co_yield true;
		co_yield false;
	}
}

CTL_NAMESPACE_END

#endif