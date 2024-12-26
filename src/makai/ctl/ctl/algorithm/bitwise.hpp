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
constexpr usize highBit(usize n) {
	return 1 << (bitWidth(n)-1);
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_BITWISE_H
