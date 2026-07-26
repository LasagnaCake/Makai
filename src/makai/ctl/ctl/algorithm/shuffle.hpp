#ifndef CTL_ALGORITHM_SHUFFLE_H
#define CTL_ALGORITHM_SHUFFLE_H

#include "../container/iterator.hpp"
#include "transform.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Shuffles the given range of elements.
/// @tparam T Iterator type.
/// @param begin Iterator to beginning of range.
/// @param end Iterator to end of range.
/// @param f Shuffling function to use.
template<Type::ReferenceIterator T, Type::Functional<usize(usize)> TFunc>
constexpr void shuffle(T const& begin, T const& end, TFunc const& f) {
	for(usize i = 0; i < (static_cast<usize>(end - begin)/2); ++i)
		swap(*(begin + i), *(end - f(i) - 1));
}

/// @brief Shuffles the given range of elements.
/// @tparam T Element type.
/// @param arr Pointer to beginning of range.
/// @param sz Size of range.
/// @param f Shuffling function to use.
template<class T, Type::Functional<usize(usize)> TFunc>
constexpr void shuffle(ref<T> const arr, usize const sz, TFunc const& f) {
	shuffle(arr, arr + sz, f);
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_SHUFFLE_H
