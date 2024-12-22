#ifndef CTL_ASYNC_CO_CONSUMER_H
#define CTL_ASYNC_CO_CONSUMER_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "context.hpp"

// Based off of: https://www.scs.stanford.edu/~dm/blog/c++-coroutines.html#compiling-code-using-coroutines

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Coroutine promise consumer.
	/// @tparam TPromise Promise type.
	template<class TPromise>
	struct Consumer {
		TPromise& promise;
		constexpr bool await_ready()							{return false;								}
		constexpr bool await_suspend(Context<TPromise> context)	{promise = context.promise(); return false;	}
		constexpr TPromise& await_resume()						{return promise;							}
	};

	/// @brief Returns the underlying STL promise type for a given promise.
	/// @tparam T Promise type.
	template<class T>
	using Unpack = Consumer<typename T::promise_type>;
}

CTL_NAMESPACE_END

#endif