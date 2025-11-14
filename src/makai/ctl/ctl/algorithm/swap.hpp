#ifndef CTL_ALGORITHM_SWAP_H
#define CTL_ALGORITHM_SWAP_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

namespace Type::Algorithm {
	template <class T>
	concept HasOwnSwap = requires (T a, T b) {
		{T::swap(a, b)};
	};
}

/// @brief Swaps two values.
/// @tparam T Value type.
/// @param a Value to swap.
/// @param b Value to swap with.
template<class T>
constexpr void swap(T& a, T& b) noexcept {
	if constexpr (Type::Algorithm::HasOwnSwap<T>) {
		T::swap(a, b);
	} else {
		T tmp = a;
		a = b;
		b = tmp;
	}
}

CTL_NAMESPACE_END

#endif