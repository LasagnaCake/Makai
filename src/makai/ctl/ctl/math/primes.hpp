#ifndef CTL_MATH_PRIMES_H
#define CTL_MATH_PRIMES_H

#include "core.hpp"

#include "../container/lists/list.hpp"
#include "../random/mersenne.hpp"
#include "../random/ctprng.hpp"
#include "../algorithm/hash.hpp"


CTL_NAMESPACE_BEGIN

/// @brief Math functions.
namespace Math {
	/// @brief Implementations.
	namespace Impl {
		/// @brief Checks to see if a value is contained inside an array.
		/// @tparam T Unsigned integer type.
		/// @tparam N Array size.
		/// @param v Value to check.
		/// @param arr Array to check against.
		/// @return Whether the array contains the value.
		template<Type::Unsigned T, usize N>
		constexpr bool contains(T const v, As<T[N]> const& arr) {
			for (T const e: arr)
				if (e == v) return true;
			return false;
		}

		/// @brief Shifts the array's contents towards the beginning, then inserts a value at the end.
		/// @tparam T Unsigned integer type.
		/// @tparam N Array size.
		/// @param v Value to insert at the end.
		/// @param arr Array to shift.
		/// @return Value at the beginning of the array.
		template<Type::Unsigned T, usize N>
		constexpr T shift(T const v, As<T[N]>& arr) {
			T const out = arr[0];
			for (usize i = N; i > 0; --i)
				arr[i-1] = arr[i];
			arr[N-1] = v;
			return out;
		}
	}

	/// @brief Returns whether a number is a prime, with a given range of certainty.
	/// @tparam T Unsigned integer type. By default, it is `usize`.
	/// @tparam TProbability Probability result type. By default, it is `float`.
	/// @tparam MEMORY Max unique bases to remember. By default, it is 4.
	/// @param value Number to check for.
	/// @param accuracy Maximum precision. By default, it is equal to `sizeof(usize) * 8`.
	/// @return Probability of number being a prime, in the range of [0, 1].
	/// @note
	///		For sufficiently small values (less than 262),
	///		this check will always be deterministic.
	/// @details
	///		How it works:
	///
	///		- If number is 0, it is not a prime.
	///
	///		- If number is 1, 2, or 3, it is a prime.
	///
	///		- Checks if number is divisible by the first 32 primes,
	///			up until a prime larger than the number is found.
	///
	///		-	- If number is found to be divisible by any, it is not a prime.
	///
	///		-	- If number is one of the primes, it is a prime.
	///
	///		- If not divisible by the first 32 primes, and is less than
	///			2x the 32nd prime (131), then it is a prime.
	///
	///		- Does modulo exponentiation with a set of random bases,
	///			With `value-1` as the exponent, and `value` as the modulo.
	///			The amount of times done is equal to `accuracy`.
	///			Then, returns the amount of operations that resulted in 1, divided by the accuracy.
	template<Type::Unsigned T = usize, Type::Float TProbability = float, usize MEMORY = 4>
	constexpr TProbability isPossiblePrime(T const value, usize const accuracy = sizeof(usize) * 8) {
		Random::Engine::Impl::Mersenne rng(Random::CTPRNG<usize>);
		constexpr As<T[]> FIRST_PRIMES = {
			2, 3, 5, 7, 11, 13, 17, 19,
			23, 29, 31, 37, 41, 43, 47, 53,
			59, 61, 67, 71, 73, 79, 83, 89,
			97, 101, 103, 107, 109, 113, 127, 131
		};
		if (!value) return 0;
		if (value < 4) return 1;
		for (usize i = 0; i < 32; ++i) {
			auto const factor = FIRST_PRIMES[i];
			if (value == factor)		return 1;
			if (value < factor)			break;
			if (value % factor == 0)	return 0;
		}
		if (value < FIRST_PRIMES[31] * 2) return 1;
		usize certainty = 0;
		T last[MEMORY] = {0};
		for (usize i = 0; i < accuracy; ++i) {
			T const factor = (rng.next() % (value - 3)) + 2;
			if (Impl::contains(factor, last)) {
				--i;
				continue;
			}
			Impl::shift(factor, last);
			if (uipowm<T>(factor, value-1, value) == 1)
				++certainty;
		}
		return certainty / static_cast<TProbability>(accuracy);
	}
	static_assert(static_cast<int>(isPossiblePrime<usize>(31)) == 1);
	static_assert(static_cast<int>(isPossiblePrime<usize>(32)) == 0);
}

CTL_NAMESPACE_END

#endif