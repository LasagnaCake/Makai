#ifndef MAKAILIB_GRAPH_COLOR_MIX_H
#define MAKAILIB_GRAPH_COLOR_MIX_H

#include "../../compat/ctl.hpp"
#include "../../file/json.hpp"

/// @brief Color mixing facilities.
namespace Makai::Graph::Color::Mix {
	constexpr Vector4 linear(Vector4 const& a, Vector4 const& b, float const weight = 0.5) {
		return Math::lerp<Vector4>(a, b, weight);
	}

	constexpr Vector4 additive(Vector4 const& a, Vector4 const& b, float const weight = 0.5) {
		return a * weight + b * (1 - weight);
	}

	constexpr Vector4 subtractive(Vector4 const& a, Vector4 const& b, float const weight = 0.5) {
		return (1 - a * weight) * (1 - b * (1 - weight));
	}

	constexpr Vector4 pigment(Vector4 const& a, Vector4 const& b, float const weight = 0.5) {
		return 0;
	}
}

#endif
