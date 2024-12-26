#ifndef CTL_ASYNC_CO_CONTEXT_H
#define CTL_ASYNC_CO_CONTEXT_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "../../templates.hpp"
#include "../../order.hpp"
#include "../../typetraits/traits.hpp"

#include <coroutine>

// For future reference: https://github.com/gcc-mirror/gcc/blob/7d83a32aacd6005c0c038c74562e35d70f6a77a8/libstdc%2B%2B-v3/include/std/coroutine#L264

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Coroutine context. Basically a `std::coroutine_handle`.
	/// @tparam TReturn Promise type.
	template<class TPromise = void>
	using Context = std::coroutine_handle<TPromise>;
}

CTL_NAMESPACE_END

#endif