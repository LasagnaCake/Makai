#ifndef CTL_EX_COLLISION_COLLISION2D_AREA_H
#define CTL_EX_COLLISION_COLLISION2D_AREA_H

#include "bounds.hpp"
#include "withinbounds.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Two-dimensional collision.
namespace Collision::C2D {
	namespace {
		using
			Math::center,
			Math::Transform2D,
			Math::Vector2
		;
	}

	/// @brief Collision layer mask.
	typedef BitMask<uint16, 4> LayerMask;

	/// @brief Collision event direction.
	enum class Direction {
		CD_NONE			= 0b00,
		CD_FORWARDS		= 0b01,
		CD_BACKWARDS	= 0b10,
		CD_BOTH			= 0b11
	};
	/*enum class Direction {
		CD_FORWARDS		= enumcast(StandardOrder::LESS),
		CD_BOTH			= enumcast(StandardOrder::EQUAL),
		CD_BACKWARDS	= enumcast(StandardOrder::GREATER),
		CD_NONE			= enumcast(StandardOrder::UNORDERED)
	};*/

	/// @brief Combines two collision events into a single `Direction`.
	/// @param forwards Result of a forwards collision event.
	/// @param backwards Result of a backwards collision event.
	/// @return Both events as a single `Direction`.
	constexpr Direction asDirection(bool const forwards, bool const backwards) {
		return Direction(
			usize(forwards)
		|	(usize(backwards) << 1)
		);
	}
	
	/// @brief Collision area.
	struct Area {
		/// @brief Shape of the collision area.
		Instance<IBound2D>	shape;
		/// @brief Whether collision is enabled for the area.
		bool				enabled	= true;
		/// @brief Collision layers this area affects.
		LayerMask			affects		= LayerMask(true);
		/// @brief Collision layers this area is affected by.
		LayerMask			affectedBy	= LayerMask(true);

		/// @brief Checks collision with another `Area`.
		/// @param other `Area` to check against.
		/// @return Collision event direction.
		constexpr Direction colliding(Area const& other) const {
			return check(*this, other);
		}

		/// @brief Checks collision between two areas.
		/// @param a `Area` to check.
		/// @param b `Area` to check against.
		/// @return Collision event direction.
		/// @note Directions:
		///
		///		- Forward:	A --> B
		///
		///		- Backward:	A <-- B
		///
		///		- Both:		A <-> B
		constexpr static Direction check(Area const& a, Area const& b) {
			if (!(a.shape && b.shape))
				return Direction::CD_NONE;
			if (!(a.enabled && b.enabled))
				return Direction::CD_NONE;
			Direction dir = asDirection(
				a.affects.match(b.affectedBy).overlap(),
				b.affectedBy.match(a.affects).overlap()
			);
			if (
				dir != Direction::CD_NONE
			&&	withinBounds(*a.shape, *b.shape)
			) return dir;
			return Direction::CD_NONE;
		}

		/// @brief Checks collision between two areas.
		/// @param a `Area` to check.
		/// @param b `Area` to check against.
		/// @return Collision event direction.
		/// @note Directions:
		///
		///		- Forward:	A --> B
		///
		///		- Backward:	A <-- B
		///
		///		- Both:		A <-> B
		constexpr friend Direction operator<=>(Area const& a, Area const& b) {
			return check(a, b);
		}
	};
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_AREA_H
