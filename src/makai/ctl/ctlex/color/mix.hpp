#ifndef CTL_EX_COLOR_MIX_H
#define CTL_EX_COLOR_MIX_H

#include "core.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Color mixing facilities.
namespace Color::Mix {
	constexpr RGBAF linear(RGBAF const& a, RGBAF const& b, float const weight = 0.5) {
		return CTL::Math::lerp<RGBAF>(a, b, weight);
	}

	constexpr RGBAF additive(RGBAF const& a, RGBAF const& b, float const weight = 0.5) {
		return a * weight + b * (1 - weight);
	}

	constexpr RGBAF subtractive(RGBAF const& a, RGBAF const& b, float const weight = 0.5) {
		return (1 - a * weight) * (1 - b * (1 - weight));
	}

	constexpr RGBAF pigment(RGBAF const& a, RGBAF const& b, float const weight = 0.5) {
		return 0;
	}
}

CTL_EX_NAMESPACE_END

#endif
