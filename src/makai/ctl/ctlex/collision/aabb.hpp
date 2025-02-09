#ifndef CTL_EX_COLLISION_AABB_H
#define CTL_EX_COLLISION_AABB_H

#include "../../ctl/ctl.hpp"
#include "../math/vector.hpp"

#include "aabb.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Collision detection facilities.
namespace Collision {
	namespace {
		using Math::Vector;
	}
	
	/// @brief Axis-aligned bounding box.
	/// @tparam D Dimension.
	template<usize D>
	struct AABB {
		static_assert(D > 1, "AABB is only available for 2D and up!");

		constexpr static usize DIMENSION = D;

		Vector<D> min;
		Vector<D> max;

		template<usize DO = D>
		constexpr bool overlap(AABB<DO> const& other) const {
			if constexpr (DO == D)		return (overlapMin(other) || overlapMax(other));
			else if constexpr (DO > D)	return overlap({other.min, other.max});
			else						return other.overlap({min, max});
		}

		constexpr AABB normalized() const {
			return {min.min(max), max.max(min)};
		}
	
	private:
		constexpr bool overlapMin(AABB const& other) const {
			bool overlapping = true;
			overlapping = overlapping && (other.min.x <= min.x && min.x <= other.max.x);
			overlapping = overlapping && (other.min.y <= min.y && min.y <= other.max.y);
			if constexpr (D > 2)
				overlapping = overlapping && (other.min.z <= min.z && min.z <= other.max.z);
			if constexpr (D > 3)
				overlapping = overlapping && (other.min.w <= min.w && min.w <= other.max.w);
			return overlapping;
		}

		constexpr bool overlapMax(AABB const& other) const {
			bool overlapping = true;
			overlapping = overlapping && (other.min.x <= max.x && max.x <= other.max.x);
			overlapping = overlapping && (other.min.y <= max.y && max.y <= other.max.y);
			if constexpr (D > 2)
				overlapping = overlapping && (other.min.z <= max.z && max.z <= other.max.z);
			if constexpr (D > 3)
				overlapping = overlapping && (other.min.w <= max.w && max.w <= other.max.w);
			return overlapping;
		}
	};
}

CTL_EX_NAMESPACE_END

#endif