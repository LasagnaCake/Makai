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
	
	static_assert(Point(0).bounded(Point(0)));
	static_assert(Point(0).bounded(Circle(0, 1)));
	static_assert(Point(0).bounded(Box(0, 1)));
	static_assert(Circle(0, 1).bounded(Circle(0, 1)));
	static_assert(Circle(0, 1).bounded(Box(0, 1)));
	static_assert(Box(0, 1).bounded(Box(0, 1)));
	
	static_assert(withinBounds(Point(0), Point(0)));
	static_assert(withinBounds(Point(0), Circle(0.5, 1)));
	static_assert(withinBounds(Point(0), Box(0, 1)));
	static_assert(withinBounds(Circle(0.5, 1), Circle(0, 1)));
	static_assert(withinBounds(Circle(0.5, 1), Box(0, 1)));
	static_assert(withinBounds(Box(0.25, 1), Box(-0.25, 1)));
	
	static_assert(withinBounds(Point(0 + 64), Point(0 + 64)));
	static_assert(withinBounds(Point(0 + 64), Circle(0.5 + 64, 1)));
	static_assert(withinBounds(Point(0 + 64), Box(0 + 64, 1)));
	static_assert(withinBounds(Circle(1 + 64, 2), Circle(0 + 64, 2)));
	static_assert(withinBounds(Circle(1 + 64, 2), Box(0 + 64, 2)));
	static_assert(withinBounds(Box(0.25 + 64, 1), Box(-0.25 + 64, 1)));
	
	static_assert(!withinBounds(Point(-1), Point(1)));
	static_assert(!withinBounds(Point(-1), Circle(1, 1)));
	static_assert(!withinBounds(Circle(-1, 1), Circle(1, 1)));
	static_assert(!withinBounds(Circle(-1, 1), Box(1, 1)));
	static_assert(!withinBounds(Box(-2, 1), Box(2, 1)));
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_WITHINBOUNDS_H
