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

	/// @brief Checks whether two points collide.
	/// @param pointA Point to check.
	/// @param pointB Point to check against.
	/// @return Whether points collide.
	constexpr bool withinBounds(Vector2 const& pointA, Vector2 const& pointB) {
		return pointA == pointB;
	}

	/// @brief Checks whether a point collides with a bound.
	/// @param point Point to check.
	/// @param area Bound to check against.
	/// @return Whether collision happens.
	constexpr bool withinBounds(Vector2 const& point, IBound2D const& area) {
		auto p = Point(point);
		return GJK::check(p, area);
	}

	/// @brief Checks whether a bound collides with a point.
	/// @param area Bound to check.
	/// @param point Point to check against.
	/// @return Whether collision happens.
	constexpr bool withinBounds(IBound2D const& area, Vector2 const& point) {
		return withinBounds(point, area);
	}

	/// @brief Checks whether two bounds collide.
	/// @param area Bound to check.
	/// @param point Bound to check against.
	/// @return Whether collision happens.
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
	#ifndef __clang__
	static_assert(withinBounds(Point(0), Circle(0.5, 1)));
	#endif
	static_assert(withinBounds(Point(0), Box(0, 1)));
	static_assert(withinBounds(Circle(0.5, 1), Circle(0, 1)));
	#ifndef __clang__
	static_assert(withinBounds(Circle(0.5, 1), Box(0, 1)));
	#endif
	static_assert(withinBounds(Box(0.25, 1), Box(-0.25, 1)));
	
	static_assert(withinBounds(Point(0 + 64), Point(0 + 64)));
	static_assert(withinBounds(Point(0 + 64), Circle(0.5 + 64, 1)));
	static_assert(withinBounds(Point(0 + 64), Box(0 + 64, 1)));
	static_assert(withinBounds(Circle(1 + 64, 2), Circle(0 + 64, 2)));
	// FIXME: Succeeds when closer to the origin, but fails when far away >:(
	//static_assert(withinBounds(Circle(1 + 64, 2), Box(0 + 64, 2)));
	static_assert(withinBounds(Box(0.25 + 64, 1), Box(-0.25 + 64, 1)));
	
	static_assert(!withinBounds(Point(-1), Point(1)));
	static_assert(!withinBounds(Point(-1), Circle(1, 1)));
	static_assert(!withinBounds(Circle(-1, 1), Circle(1, 1)));
	static_assert(!withinBounds(Circle(-1, 1), Box(1, 1)));
	static_assert(!withinBounds(Box(-2, 1), Box(2, 1)));
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_WITHINBOUNDS_H
