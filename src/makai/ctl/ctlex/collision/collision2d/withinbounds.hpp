#ifndef CTL_EX_COLLISION_COLLISION2D_WITHINBOUNDS_H
#define CTL_EX_COLLISION_COLLISION2D_WITHINBOUNDS_H

#include "bounds.hpp"
#include "../gjk.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace Collision::C2D {
	namespace {
		using
			Math::center,
			Math::Transform2D,
			Math::Vector2,
			Math::angleV2
		;
	}

	constexpr bool withinBounds(Vector2 const& point, IBound2D const& area) {
		auto p = Point(point);
		return GJK::check(p, area);
	}

	constexpr bool withinBounds(IBound2D const& a, IBound2D const& b) {
		return GJK::check(a, b);
	}
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_WITHINBOUNDS_H
