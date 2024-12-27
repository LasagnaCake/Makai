#ifndef CTL_ASYNC_CO_CONSUMER_H
#define CTL_ASYNC_CO_CONSUMER_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "awaitable.hpp"

// Based off of: https://www.scs.stanford.edu/~dm/blog/c++-coroutines.html#compiling-code-using-coroutines

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Single-pass awaiter.
	struct Consumer: IAwaitable<void, void> {
		bool await_ready() final	{return consume();	}
		void await_suspend() final	{					}
		void await_resume() final	{					}
	protected:
		/// @brief What to do when entering the wait.
		virtual void onEnter()	{};
		/// @brief What to do when exiting the wait.
		virtual void onExit()	{};
	private:
		bool consumed = false;
		constexpr bool consume() {
			if (!consumed) {
				onEnter();
				return consumed = true;
			}
			onExit();
			return false;
		}
	};
}

CTL_NAMESPACE_END

#endif