#ifndef CTL_EX_MATH_INTERPOLATE_H
#define CTL_EX_MATH_INTERPOLATE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/math/core.hpp"
#include "vector.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Math facilities.
namespace Math {
	template <::CTL::Type::Math::Operatable T, usize D>
	constexpr static T interpolate(As<T const[1 << D]> const& data, As<T const[D]> const& amount) {
		if constexpr (D == 1)
			return ::CTL::Math::lerp(data[0], data[1], amount[0]);
		else {
			constexpr auto const NEW_SIZE = 1 << (D-1);
			As<T[NEW_SIZE]> buf;
			for (usize i = 0; i < NEW_SIZE; ++i)
				buf[i] = ::CTL::Math::lerp(data[i], data[i + NEW_SIZE], amount[D-1]);
			return interpolate<D-1>(buf, amount);
		}
	}

	template <class T>
	constexpr T blerp(As<T const[4]> const& data, As<T const[2]> const& amount) {
		return interpolate<2>(data, amount);
	}

	template <class T>
	constexpr T tlerp(As<T const[8]> const& data, As<T const[3]> const& amount) {
		return interpolate<3>(data, amount);
	}
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_MATH_INTERPOLATE_H
