#ifndef CTL_EX_ASYNC_WAITER_H
#define CTL_EX_ASYNC_WAITER_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/async/atomic.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Thread timer.
class Ticker: public SelfIdentified<Ticker> {
	using Counter = Atomic<usize>;

public:
	/// @brief Thread timer waiter.
	class Waiter: public SelfIdentified<Waiter>, Async::Base::Yieldable {
		/// @brief Copy constructor.
		/// @param other Waiter to copy from.
		constexpr Waiter(Waiter const& other): counter(other.counter)	{}
		/// @brief Copy constructor.
		/// @param other Waiter to move.
		constexpr Waiter(Waiter&& other): counter(other.counter)		{}

		/// @brief Destructor.
		constexpr ~Waiter() {}

		/// @brief Sleeps the thread it is called in for a given amount of ticks.
		/// @param ticks Amount of ticks to wait for.
		void wait(usize const ticks) {
			usize delay = counter;
			while (counter.value() < (delay + ticks)) asyncYield();
		}

	private:
		constexpr Waiter(Counter& counter): counter(counter)	{}

		/// @brief Counter bound to the waiter.
		Counter& counter;

		friend class Ticker;
	};

	/// @brief Empty constructor.
	Ticker()	{}
	/// @brief Destructor.
	~Ticker()	{}

	/// @brief Yields the ticker.
	void yield()	{++counter;					}
	/// @brief Creates a waiter for the ticker.
	Waiter waiter()	{return Waiter(counter);	}

private:
	/// Underlying counter.
	Counter counter{0};
};

CTL_EX_NAMESPACE_END

#endif // CTL_EX_ASYNC_WAITER_H
