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
		bool				canCollide	= true;
		/// @brief Tags associated with the collision object.
		LayerMask			tags		= LayerMask(false);

		/// @brief Checks collision with another `Area`.
		/// @param other `Area` to check against.
		/// @return Whether collision happens.
		constexpr bool colliding(Area const& other) const {
			return check(*this, other);
		}

		/// @brief Checks collision between two areas.
		/// @param a `Area` to check.
		/// @param b `Area` to check against.
		/// @return Whether collision happens.
		constexpr static bool check(Area const& a, Area const& b) {
			if (a.canCollide && b.canCollide)
				return withinBounds(*a.shape, *b.shape);
			return false;
		}
	};
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_AREA_H
