#ifndef CTL_ASYNC_WAITABLE_H
#define CTL_ASYNC_WAITABLE_H

#include "../namespace.hpp"
#include "../container/functor.hpp"
#include "thread.hpp"
#include "atomic.hpp"
#include "../container/error.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Asynchronous-related errors
namespace Error::Async {
	DEFINE_ERROR_TYPE(Occupied);
}

/// @brief Asynchronous facilities.
namespace Async {
	/// @brief Sleeps the thread it is called in, for a specific amount of time.
	/// @param time Time to sleep for.
	inline void wait(usize const millis) {
		Thread::wait(millis);
	}

	/// @brief Makes the thread it is called in wait for a condition to be met.
	/// @param condition Condition to wait for.
	inline void wait(Atomic<bool>& condition) {
		while (condition.value()) Thread::yield();
	}

	/// @brief Makes the thread it is called in wait for a predicate function to be true.
	/// @param predicate Condition to wait for.
	inline void wait(Atomic<Trigger<>>& predicate) {
		while (predicate.value()()) Thread::yield();
	}

	/// @brief Makes the thread it is called in wait for a predicate function to be true.
	/// @param predicate Condition to wait for.
	inline void wait(Trigger<> const& predicate) {
		while (predicate()) Thread::yield();
	}
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_WAITABLE_H
