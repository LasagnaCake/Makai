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

	/// @brief Maximum detection precision.
	constexpr float const PRECISION = 1e-6;
	
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
			return 
				(contains(other.min) || contains(other.max))
			||	(other.contains(min) || other.contains(max))
			;
		}

		/// @brief Returns whether this bounding box perfectly overlaps with another.
		/// @tparam DO Other bounding box's dimension.
		/// @param other Bounding box to check overlap with.
		/// @return Whether boxes perfectly overlap.
		template<usize DO = D>
		constexpr bool match(AABB<DO> const& other) const {
			return CTL::Math::compare<float>(coverage(other), 1.0, PRECISION);
		}

		/// @brief Returns the bounding box's size.
		/// @return Bounding box size.
		constexpr Vector<D> size() const {return (max - min).absolute();}

		/// @brief Returns the bounding box's center.
		/// @return Bounding box center.
		constexpr Vector<D> center() const {return (max + min) / 2.0f;}

		/// @brief Returns how much overlap exists between bounding boxes.
		/// @tparam DO Other bounding box's dimension.
		/// @param other Bounding box to get overlap with.
		/// @return How much boxes overlap. Returns -1 if it is impossible to determine (i.e. when both bound's sizes are zero).
		template<usize DO = D>
		constexpr float coverage(AABB<DO> const& other) const {
			if (min == other.min && max == other.max)
				return 1;
			auto const dmax = max.min(other.max);
			auto const dmin = min.max(other.min);
			auto const as = size();
			auto const bs = other.size();
			auto const ds = (dmax - dmin).absolute();
			float const da = (ds.x * ds.y);
			float const ua = ((as.x * as.y + bs.x * bs.y) - da);
			if (!ua) return -1;
			if (!da) return 0;
			return CTL::Math::clamp<float>(da/ua, 0, 1);
		}

		/// @brief Returns the bounding box configured correctly.
		/// @return Correctly-configured bounding box.
		constexpr AABB normalized() const {
			return {min.min(max), max.max(min)};
		}

		/// @brief Returns whether the given point is inside the bounding box.
		/// @param point Point to check.
		/// @return Whether point is inside bounding box.
		template<usize DO = D>
		constexpr static bool contains(Vector<DO> const& point, Vector<DO> const& min, Vector<DO> const& max) {
			if constexpr (DO == 2) return 
				(min.x <= point.x && point.x <= max.x)
			||	(min.y <= point.y && point.y <= max.y)
			;
			else if constexpr (DO == 3) return 
				(min.x <= point.x && point.x <= max.x)
			||	(min.y <= point.y && point.y <= max.y)
			||	(min.z <= point.z && point.z <= max.z)
			;
			else if constexpr (DO == 3) return 
				(min.x <= point.x && point.x <= max.x)
			||	(min.y <= point.y && point.y <= max.y)
			||	(min.z <= point.z && point.z <= max.z)
			||	(min.w <= point.w && point.w <= max.w)
			;
		}

		/// @brief Returns whether the given point is inside the bounding box.
		/// @param point Point to check.
		/// @return Whether point is inside bounding box.
		template<usize DO = D>
		constexpr bool contains(Vector<DO> const& point) const {
			if constexpr (D < DO)	return contains<DO>(point, min, max);
			else					return contains<D>(point, min, max);
		}
	
	private:
	};
}

CTL_EX_NAMESPACE_END

#endif