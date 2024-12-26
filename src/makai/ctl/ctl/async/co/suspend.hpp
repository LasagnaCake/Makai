#ifndef CTL_ASYNC_CO_SUSPEND_H
#define CTL_ASYNC_CO_SUSPEND_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "context.hpp"

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

	/// @brief Dynamic suspension indicator.
	struct DynamicSuspend {
		bool const awaiting = false;
		virtual bool await_ready()		{return awaiting;	}
		virtual void await_suspend()	{					}
		virtual void await_resume()		{					}
	};
}

CTL_NAMESPACE_END

#endif