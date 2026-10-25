#ifndef CTL_EX_TWEENING_INTERPOLATE_H
#define CTL_EX_TWEENING_INTERPOLATE_H

#include "../../ctl/exnamespace.hpp"
#include "../math/ease.hpp"
#include "tweenable.hpp"

CTL_EX_NAMESPACE_BEGIN

template<Type::Ex::Tween::Tweenable T>
constexpr T interpolate(T const& from, T const& to, float factor, Math::Ease::Mode const& mode = Math::Ease::linear) {
	factor = CTL::Math::clamp<float>(factor, 0, 1);
	if (factor == 0)	return from;
	if (factor < 1)		return CTL::Math::lerp<T>(from, to, mode(factor));
	return to;
}

template <Type::Ex::Tween::Tweenable T>
using Interpolator = Function<T(T const&, T const&, float const)>;

template<Type::Ex::Tween::Tweenable T>
constexpr Interpolator<T> interpolator(Math::Ease::Mode const& mode = Math::Ease::linear) {
	return [mode] (T const& from, T const& to, float const factor) {
		interpolate<T>(from, to, factor, mode);
	};
}

CTL_EX_NAMESPACE_END

#endif
