#ifndef CTL_EX_COLLISION_GJK_H
#define CTL_EX_COLLISION_GJK_H

#include "../../ctl/ctl.hpp"
#include "../math/vector.hpp"

// Based off of https://winter.dev/articles/gjk-algorithm

// TODO: Document this

CTL_EX_NAMESPACE_BEGIN

/// @brief Collision detection facilities.
namespace Collision {

/// @brief Implementation of the Gilbert-Johnson-Keerthi (GJK) algorithm for collision detection.
namespace GJK {
	namespace {
		using
			Math::Vector,
			Math::Vector2,
			Math::Vector3,
			Math::Vector4
		;
	}

	/// @brief GJK-enabled bound interface.
	/// @tparam D Dimension.
	template<usize D>
	struct IGJKBound {
		constexpr virtual ~IGJKBound() {}
		constexpr virtual Vector<D> furthest(Vector<D> const& direction) const = 0;
	};
	
	/// @brief Simplex for bound calculation.
	/// @tparam D Dimension.
	template<usize D>
	struct Simplex {
		static_assert(D > 0 && D < 5);

		/// @brief Dimension of the simplex.
		constexpr static usize DIMENSION	= D;
		/// @brief Maximum amount of points in the simplex.
		constexpr static usize MAX_POINTS	= DIMENSION+1;

		/// @brief Vector type.
		using VectorType = Vector3;
		/// @brief Point array type.
		using PointArrayType = Array<VectorType, D+1>;
		//using VectorType = Vector<DIMENSION>;

		/// @brief Empty constructor.
		constexpr Simplex(): count(0) {}

		/// @brief Constructs the simplex from a list of points.
		/// @param ps Points to construct from.
		constexpr Simplex(List<VectorType> const& ps): Simplex() {
			for (;count < ps.size() && count < MAX_POINTS; ++count)
				points[count] = ps[count];
		}
		
		/// @brief Returns an iterator to the beginning of the point list.
		/// @return iterator to beginning of point list.
		constexpr Iterator<VectorType const>	begin() const	{return points.begin();							}
		/// @brief Returns an iterator to the end of the point list.
		/// @return iterator to end of point list.
		constexpr Iterator<VectorType const>	end() const		{return points.end() - (MAX_POINTS - count);	}
		/// @brief Returns a pointer to the underlying point list.
		/// @return Pointer to underlying point list.
		constexpr VectorType const*				data() const	{return points.data();							}
		/// @brief Returns the ammount of points the simplex has.
		/// @return Point count.
		constexpr usize							size() const	{return count;									}

		/// @brief Returns the value of the point at the given index.
		/// @param index Index of point to get.
		/// @return Value of point.
		/// @throw OutOfBoundsException if simplex is empty.
		/// @throw Whatever `Array::operator[]` throws.
		constexpr VectorType operator[](ssize const index) const {
			if (count == 0)
				throw OutOfBoundsException("Simplex is empty!");
			return points[index];
		}

		/// @brief Adds a point to the front of the simplex.
		/// @param vec Point to add.
		/// @return Reference to self.
		constexpr Simplex& pushFront(VectorType const& vec) {
			points = {vec, points[0], points[1], points[2]};
			if (count < MAX_POINTS)
				++count;
			return *this;
		}

		/// @brief Remakes the simplex as the next simplex to check.
		/// @param direction Direction to remake simplex for.
		/// @return Whether simplex contains the origin.
		constexpr bool remake(VectorType& direction) {
			switch (count) {
				case 2: return line(direction);
				case 3: return triangle(direction);
				case 4: return tetrahedron(direction);
			}
			return false;
		}

		/// @brief Checks if the dot product of two vectors are bigger than zero.
		/// @param direction Direction to check.
		/// @param ao Direction to check.
		/// @return Whether the dot product is bigger than zero.
		constexpr static bool same(VectorType const& direction, VectorType const& ao) {
			return direction.dot(ao) > 0;
		}

	private:
		/// @brief Simplex points.
		PointArrayType	points;
		/// @brief Point count.
		usize			count;

		constexpr bool line(VectorType& direction) {
			VectorType a = points[0];
			VectorType b = points[1];
			VectorType ab = b - a;
			VectorType ao =   - a;
			if (same(ab, ao))
				direction = ab.cross(ao).cross(ab);
			else {
				points = {a, 0, 0, 0};
				direction = ao;
			}
			return DIMENSION == 1;
		}

		constexpr bool triangle(VectorType& direction) {
			VectorType a = points[0];
			VectorType b = points[1];
			VectorType c = points[2];
			VectorType ab = b - a;
			VectorType ac = c - a;
			VectorType ao =   - a;
			VectorType abc = ab.cross(ac);
			if (same(abc.cross(ac), ao)) {
				if (same(ac, ao)) {
					points = {a, c, 0, 0};
					direction = ac.cross(ao).cross(ac);
				} else {
					points = {a, b, 0, 0};
					return line(direction);
				}
			} else if (same(ab.cross(abc), ao)) {
				points = {a, b, 0, 0};
				return line(direction);
			} else if (same(abc, ao)) {
				direction = abc;
			} else {
				points = {a, c, b, 0};
				direction = -abc;
			}
			return DIMENSION == 2;
		}

		constexpr bool tetrahedron(VectorType& direction) {
			VectorType a = points[0];
			VectorType b = points[1];
			VectorType c = points[2];
			VectorType d = points[3];
			VectorType ab = b - a;
			VectorType ac = c - a;
			VectorType ad = d - a;
			VectorType ao =   - a;
			VectorType abc = ab.cross(ac);
			VectorType acd = ac.cross(ad);
			VectorType adb = ad.cross(ab);
			if (same(abc, ao)) {
				points = {a, b, c, 0};
				return triangle(direction);
			}
			if (same(acd, ao)) {
				points = {a, c, d, 0};
				return triangle(direction);
			}
			if (same(adb, ao)) {
				points = {a, d, b, 0};
				return triangle(direction);
			}
			return DIMENSION == 3;
		}
	};

	/// @brief Gets the support vector between two bounds.
	/// @tparam D Dimension.
	/// @param a GJK-enabled bound.
	/// @param b GJK-enabled bound.
	/// @param direction Direction to get support vector for.
	/// @return Support vector.
	template<usize D>
	constexpr Vector<D> support(
		IGJKBound<D> const& a,
		IGJKBound<D> const& b,
		Vector<D> const& direction
	) {
		return b.furthest(direction) - b.furthest(-direction);
	}

	/// @brief Checks collision between two bounds.
	/// @tparam D Dimension.
	/// @param a GJK-enabled bound.
	/// @param b GJK-enabled bound.
	/// @return Whether they collide.
	template<usize D>
	constexpr bool check(
		IGJKBound<D> const& a,
		IGJKBound<D> const& b
	) {
		using VectorType = Vector<D>;
		VectorType sup = support(a, b, VectorType::RIGHT());
		Simplex<3> sp;
		sp.pushFront(sup);
		Vector3 d = -sup;
		while (true) {
			sup = support<D>(a, b, d);
			if (sup.dot(d) <= 0)
				return false;
			sp.pushFront(sup);
			if (sp.remake(d)) {
				return true;
			}
		}
	}
}

/// @brief Collision bound interface.
/// @tparam D Dimension.
template<usize D>
using IBound = GJK::IGJKBound<D>;

}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_GJK_H