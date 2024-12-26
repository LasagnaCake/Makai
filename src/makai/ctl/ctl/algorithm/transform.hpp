#ifndef CTL_ALGORITHM_TRANSFORM_H
#define CTL_ALGORITHM_TRANSFORM_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Converts the character to a lowercase one.
/// @tparam T Character type.
/// @param c Character to convert.
/// @return Character as lowercase.
template<Type::ASCII T>
constexpr T toLowerChar(T const& c) noexcept {
	if (c >= 'A' && c <= 'Z')
		return c - ('a' - 'A');
	return c;
}

/// @brief Converts the character to an uppercase one.
/// @tparam T Character type.
/// @param c Character to convert.
/// @return Character as uppercase.
template<Type::ASCII T>
constexpr T toUpperChar(T const& c) noexcept {
	if (c >= 'a' && c <= 'z')
		return c + ('a' - 'A');
	return c;
}

/// @brief Swaps two values.
/// @tparam T Value type.
/// @param a Value to swap.
/// @param b Value to swap with.
template<class T>
constexpr void swap(T& a, T& b) noexcept {
	T tmp = a;
	a = b;
	b = tmp;
}

/// @brief Reverses the given range of elements.
/// @tparam T Element type.
/// @param arr Pointer to beginning of range.
/// @param sz Size of range.
template<class T>
constexpr void reverse(T* const arr, usize const sz) noexcept {
	for(usize i = 0; i < (sz/2); ++i)
		swap(arr[i], arr[sz-1-i]);
}

/// @brief Reverses the bits for a given integer.
/// @tparam T Integer type.
/// @param data Integer to reverse.
/// @return Reversed integer.
template<Type::Integer T>
constexpr T reflect(T const& data) {
	T result = 0;
	constexpr usize BIT_WIDTH = sizeof(T) * 8;
	for (usize i = 0; i < BIT_WIDTH; i++)
		if ((data >> i) & 1) result |= (T(1) << (BIT_WIDTH-1-i));
	return (result);
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_TRANSFORM_H
