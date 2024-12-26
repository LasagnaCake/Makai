#ifndef CTL_OS_TIME_H
#define CTL_OS_TIME_H

#include <chrono>
#include "../namespace.hpp"
#include "../ctypes.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Time-related facilities.
namespace OS::Time {
	namespace {
		namespace chrono = std::chrono;
	}

	/// @brief Unit of time.
	typedef chrono::years			Years;
	/// @brief Unit of time.
	typedef chrono::months			Months;
	/// @brief Unit of time.
	typedef chrono::weeks			Weeks;
	/// @brief Unit of time.
	typedef chrono::days			Days;
	/// @brief Unit of time.
	typedef chrono::hours			Hours;
	/// @brief Unit of time.
	typedef chrono::minutes			Minutes;
	/// @brief Unit of time.
	typedef chrono::seconds			Seconds;
	/// @brief Unit of time.
	typedef chrono::milliseconds	Millis;
	/// @brief Unit of time.
	typedef chrono::microseconds	Micros;
	/// @brief Unit of time.
	typedef chrono::nanoseconds		Nanos;

	class Clock {
	private:
		typedef chrono::system_clock SysClock;
		inline static const auto start = SysClock::now();
	public:

		/// @brief Gets the current time, since the epoch (Jan 1st, 1970), in the specified unit.
		/// @tparam T Time unit.
		/// @return Time since epoch in specified unit.
		template <class T = Micros>
		constexpr static usize sinceEpoch() {
			return chrono::duration_cast<T>(SysClock::now().time_since_epoch()).count();
		}

		/// @brief Gets the current time, since program start, in the specified unit.
		/// @tparam T Time unit.
		/// @return Time since start in specified unit.
		template <class T = Micros>
		constexpr static usize sinceStart() {
			return chrono::duration_cast<T>(SysClock::now() - start).count();
		}
	};

	/// @brief Returns the current time, since the epoch, in milliseconds.
	/// @return Time in milliseconds.
	inline usize now() {
		return Clock::sinceEpoch<Millis>();
	}
}

CTL_NAMESPACE_END

#endif // CTL_OS_TIME_H
