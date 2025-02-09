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

	constexpr bool withinBounds(IBound2D const& area, Vector2 const& point) {
		return withinBounds(point, area);
	}

	constexpr bool withinBounds(IBound2D const& a, IBound2D const& b) {
		return GJK::check(a, b);
	}

	static_assert(withinBounds(Point(0), Point(0)));
	static_assert(withinBounds(0, Circle(0, 1)));
	static_assert(withinBounds(Circle(0, 1), Circle(0, 1)));
	static_assert(withinBounds(Circle(0, 1), Box(0, 1)));
	static_assert(withinBounds(Box(0, 1), Box(0, 1)));
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_WITHINBOUNDS_H
