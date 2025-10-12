#ifndef CTL_ALGORITHM_REVERSE_H
#define CTL_ALGORITHM_REVERSE_H

#include "../container/iterator.hpp"
#include "transform.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Reverses the given range of elements.
/// @tparam T Iterator type.
/// @param begin Iterator to beginning of range.
/// @param end Iterator to end of range.
template<Type::ReferenceIterator T>
void reverse(T const& begin, T const& end) {
	for(usize i = 0; i < (static_cast<usize>(end - begin)/2); ++i)
		swap(*(begin + i), *(end - i - 1));
}

/// @brief Reverses the given range of elements.
/// @tparam T Element type.
/// @param arr Pointer to beginning of range.
/// @param sz Size of range.
template<class T>
constexpr void reverse(ref<T> const arr, usize const sz) {
	reverse(arr, arr + sz);
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_REVERSE_H
