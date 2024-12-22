#ifndef CTL_ASYNC_CO_SUSPEND_H
#define CTL_ASYNC_CO_SUSPEND_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "context.hpp"

// Based off of: https://github.com/gcc-mirror/gcc/blob/7d83a32aacd6005c0c038c74562e35d70f6a77a8/libstdc%2B%2B-v3/include/std/coroutine#L264

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Always suspend indicator.
	using AlwaysSuspend	= std::suspend_always;
	/// @brief Never suspend indicator.
	using NeverSuspend	= std::suspend_never;

	/// @brief Suspension indicator.
	template<bool S>
	using Suspend = Meta::DualType<S, AlwaysSuspend, NeverSuspend>;
}

CTL_NAMESPACE_END

#endif