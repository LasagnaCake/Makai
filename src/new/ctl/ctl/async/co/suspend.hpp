#ifndef CTL_ASYNC_CO_SUSPEND_H
#define CTL_ASYNC_CO_SUSPEND_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "context.hpp"

// Based off of: https://github.com/gcc-mirror/gcc/blob/7d83a32aacd6005c0c038c74562e35d70f6a77a8/libstdc%2B%2B-v3/include/std/coroutine#L264

CTL_NAMESPACE_BEGIN

namespace Co {
	template<bool S>
	struct Suspend {
		constexpr bool await_ready()			{return S;	}
		constexpr void await_suspend(Context<>)	{			}
		constexpr void await_resume(Context<>)	{			}
	};

	using AlwaysSuspend	= Suspend<true>;
	using NeverSuspend	= Suspend<false>;

	template<class TPromise>
	struct Consumer {
		TPromise& promise;
		constexpr bool await_ready()							{return false;								}
		constexpr bool await_suspend(Context<TPromise> context)	{promise = context.promise() return false;	}
		constexpr TPromise& await_resume()						{return promise;							}
	};
}

CTL_NAMESPACE_END

#endif