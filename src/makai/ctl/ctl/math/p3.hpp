#ifndef CTL_MATH_P3_H
#define CTL_MATH_P3_H

#include "core.hpp"

CTL_NAMESPACE_BEGIN

/**
	Returns the polar radius of a point along the edges of a
	N-sided polygon of "size" S (the shape's circumradius)
	at a given angle THETA, rotated by angle A.
	Name: Polar Polygon Point -> PPP -> p3
*/
namespace Math::P3 {
	/// Precise version. Better results, but more computationally intensive.
	/// @brief Precise version. Better results, but more computationally intensive.
	/// @param theta Angle to calculate.
	/// @param sides Polygon side count.
	/// @param size Polygon size.
	/// @param angle Rotation of the polygon.
	/// @return Radius at the given angle.
	/// @note 
	///		Based off of the formula from https://www.desmos.com/calculator/hghrnwbcdh .
	///
	///		(Archive) https://web.archive.org/web/20240209160635/https://www.desmos.com/calculator/hghrnwbcdh .
	constexpr float precise(
		float const theta,
		float const sides,
		float const size	= 1,
		float const angle	= 0
	) {
		float const constant	= size * cos(PI / sides);
		float const shape		= (sides / 2.0) * theta + angle;
		return constant / cos((2.0 / sides) * asin(cos(shape)));
	}

	/// @brief General version. Not as accurate, but still good for general use.
	/// @param theta Angle to calculate.
	/// @param sides Polygon side count.
	/// @param size Polygon size.
	/// @param angle Rotation of the polygon.
	/// @return Radius at the given angle.
	constexpr float general(
		float const theta,
		float const sides,
		float const size	= 1,
		float const angle	= 0
	) {
		float const constant	= (size * SQRT3) / (sides * Math::sqrt(sides));
		float const shape		= (theta * sides + angle - HPI) / 2.0;
		return size - constant * abcos(shape);
	}

	/// @brief Fast version. Least accurate, but computationally fast.
	/// @param theta Angle to calculate.
	/// @param sides Polygon side count.
	/// @param size Polygon size.
	/// @param angle Rotation of the polygon.
	/// @return Radius at the given angle.
	constexpr float fast(
		float const theta,
		float const sides,
		float const size	= 1,
		float const angle	= 0
	) {
		float const constant	= (size * SQRT2) / (sides * 2.0);
		float const shape		= (theta * sides + angle - HPI) / 2.0;
		return size - constant * abcos(shape);
	}
}

CTL_NAMESPACE_END

#endif // CTL_MATH_P3_H
