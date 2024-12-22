#ifndef CTL_ASYNC_CO_YIELD_H
#define CTL_ASYNC_CO_YIELD_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "promise.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Coroutine staller.
	struct Yielder {
		bool await_ready()						{return !counter;	}
		bool await_suspend(Context<> context)	{return --counter;	}
		void await_resume()						{					}

		/// @brief Constructs the yielder.
		/// @param count Amount of times to stall the coroutine for.
		Yielder(usize const count): counter(count) {}

	private:
		usize counter;
	};

	/// @brief
	///		Creates a yielder that stalls a coroutine a given number of times.
	///		Meant to be used like `co_await yield(count)`.
	/// @param count Amount of times to stall the coroutine for.
	/// @return Resulting yielder.
	Yielder yield(usize const count) {return Yielder(count);}
}

CTL_NAMESPACE_END

#endif