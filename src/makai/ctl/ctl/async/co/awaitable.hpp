#ifndef CTL_ASYNC_CO_AWAITABLE_H
#define CTL_ASYNC_CO_AWAITABLE_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Awaitable object interface.
	/// @tparam TReturn Return type.
	/// @tparam TSuspend Suspend type.
	template<class TReturn, class TSuspend = bool>
	struct IAwaitable {
		/// @brief Returns whether awaiting is necessary. Must be implemented,
		/// @return Whether to await.
		virtual bool await_ready()			= 0;
		/// @brief Returns the suspension state. Must be implemented,
		/// @return Suspension state.
		virtual TSuspend await_suspend()	= 0;
		/// @brief Returns the result of the await. Must be implemented,
		/// @return Await result.
		virtual TReturn await_resume()		= 0;
	};
}

CTL_NAMESPACE_END

#endif