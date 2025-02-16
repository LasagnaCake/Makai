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

		/// @brief AABB's dimension.
		constexpr static usize DIMENSION = D;

		/// @brief Lowest corner of the bounding box.
		Vector<D> min;
		/// @brief Highest corner of the bounding box.
		Vector<D> max;

		/// @brief Returns whether this bounding box overlaps with another.
		/// @tparam DO Other bounding box's dimension.
		/// @param other Bounding box to check overlap with.
		/// @return Whether boxes overlap.
		template<usize DO = D>
		constexpr bool overlap(AABB<DO> const& other) const {
			if constexpr (DO == D)		return (overlapMin(other) || overlapMax(other));
			else if constexpr (DO > D)	return overlap({other.min, other.max});
			else						return other.overlap({min, max});
		}

		/// @brief Returns whether this bounding box perfectly overlaps with another.
		/// @tparam DO Other bounding box's dimension.
		/// @param other Bounding box to check overlap with.
		/// @return Whether boxes perfectly overlap.
		template<usize DO = D>
		constexpr bool match(AABB<DO> const& other) const {return coverage(other) > 0.9999;}

		/// @brief Returns how much overlap exists between bounding boxes.
		/// @tparam DO Other bounding box's dimension.
		/// @param other Bounding box to get overlap with.
		/// @return How much boxes overlap.
		template<usize DO = D>
		constexpr float coverage(AABB<DO> const& other) const {
			auto const a = normalized();
			auto const b = other.normalized();
			auto const max = a.max.min(b.max);
			auto const min = a.min.max(b.min);
			auto const as = (a.max - a.min).absolute();
			auto const bs = (b.max - b.min).absolute();
			auto const ds = (max - min).absolute();
			float const da = (ds.x * ds.y);
			float const ua = ((as.x * as.y + bs.x * bs.y) - da);
			if (!ua) return 1;
			if (!da) return 0;
			return CTL::Math::clamp<float>(da/ua, 0, 1);
		}

		/// @brief Returns the bounding box configured correctly.
		/// @return Correctly-configured bounding box.
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