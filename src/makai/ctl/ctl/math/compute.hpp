#ifndef CTL_MATH_COMPUTE_H
#define CTL_MATH_COMPUTE_H

#include "core.hpp"

CTL_NAMESPACE_BEGIN

namespace Math::Compute {
	template<class T, Type::Functional<T(T const&)> TFunc>
	constexpr T sum(TFunc const& f, T const& stop, T const& start = 0, T const& step = 1)
	requires Type::Math::Addable<T, T> {
		T out = {};
		for (T x = start; x < stop; x += step)
			out += f(x);
		return out;
	}

	template<class T, Type::Functional<T(T const&)> TFunc>
	constexpr T product(TFunc const& f, T const& stop, T const& start = 0, T const& step = 1)
	requires Type::Math::Addable<T, T> {
		T out = {};
		for (T x = start; x < stop; x += step)
			out *= f(x);
		return out;
	}

	template <Type::Real T>
	constexpr T pi(usize const precision = sizeof(T) * 4) {
		return 4 * sum<T>(
			[] (T const& k) -> T {
				return pow(-1, k+1) / (2 * (k-1));
			},
			precision+1,
			1
		);
	}
}

CTL_NAMESPACE_END

#endif
