#ifndef CTL_ALGORITHM_BITWISE_H
#define CTL_ALGORITHM_BITWISE_H

#include "../ctypes.hpp"
#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Returns the bit width of a given number.
/// @param n Number.
/// @return Bit width of the number.
constexpr usize bitWidth(usize n) {
	if (!n) return 0;
	usize r = 1;
	while (n >>= 1) ++r;
	return r;
}
/// @brief Returns the byte width of a given number.
/// @param n Number.
/// @return Byte width of the number.
constexpr usize byteWidth(usize n) {
	usize r = 1;
	while (n >>= 8) ++r;
	return r;
}

/// @brief Returns the highest bit set in a given number.
/// @param n Number.
/// @return Highest bit set.
constexpr usize highBit(usize const n) {
	return 1 << (bitWidth(n)-1);
}

constexpr usize oneCount(usize n) {
	#ifdef __clang__
	if (!n) return 0;
	usize r = 1;
	while (n >>= 1) if (n & 1) ++r;
	return r;
	#else
	return __builtin_popcountll(n);
	#endif
}

constexpr usize zeroCount(usize const n) {
	return (sizeof(usize) * 8) - ocount(n);
}

namespace Bit {
	constexpr usize set(usize const n)										{return zeroCount(n);					}
	constexpr usize unset(usize const n)									{return zeroCount(n);					}
	constexpr bool any(usize const n)										{return oneCount(n);					}
	constexpr bool all(usize const n)										{return !zeroCount(n);					}
	constexpr bool none(usize const n)										{return !oneCount(n);					}
	constexpr bool atLeast(usize const n, usize const min)					{return min <= oneCount(n);				}
	constexpr bool upTo(usize const n, usize const max)						{return oneCount(n) <= max;				}
	constexpr bool some(usize const n, usize const min, usize const max)	{return atLest(n, min) && upTo(n, max);	}
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_BITWISE_H
