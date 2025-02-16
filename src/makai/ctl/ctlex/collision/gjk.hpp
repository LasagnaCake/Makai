#ifndef CTL_EX_COLLISION_GJK_H
#define CTL_EX_COLLISION_GJK_H

#include "../../ctl/ctl.hpp"
#include "../math/vector.hpp"

#include "aabb.hpp"

// Based off of https://winter.dev/articles/gjk-algorithm

CTL_EX_NAMESPACE_BEGIN

namespace Type::Ex::Collision::GJK {
	template<usize N>
	concept Dimension = (N == 2 || N == 3);
	template<usize... D>
	concept Dimensions = (... && Dimension<D>);
}

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
		/// @brief Destructor.
		constexpr virtual ~IGJKBound() {}
		/// @brief Returns the furthest point in a given direction. Must be implemented.
		constexpr virtual Vector<D> furthest(Vector<D> const& direction) const	= 0;
		/// @brief Returns the Axis-Aligned Bounding Box the shape resides in. Must be implemented.
		constexpr virtual AABB<D> aabb() const									= 0;

		/// @brief Checks if this shape's AABB overlaps with another shape's AABB.
		/// @tparam DO Other shape's dimension.
		/// @param other Shape to check overlap with.
		/// @return Whether shapes overlap.
		template<usize DO>
		constexpr bool bounded(IGJKBound<DO> const& other) const	{return aabb().overlap(other.aabb());	}

		/// @brief Returns how much this shape's AABB overlaps a lot with another shape's AABB.
		/// @tparam DO Other shape's dimension.
		/// @param other Shape to get overlap with.
		/// @return How much shapes overlap.
		template<usize DO>
		constexpr bool overlap(IGJKBound<DO> const& other) const	{return aabb().overlap(other.aabb());	}

		/// @brief Checks if this shape's AABB perfectly overlaps with another shape's AABB.
		/// @tparam DO Other shape's dimension.
		/// @param other Shape to check overlap with.
		/// @return Whether shapes perfectly overlap.
		template<usize DO>
		constexpr bool match(IGJKBound<DO> const& other) const		{return aabb().match(other.aabb());		}
	};
	
	/// @brief Simplex for bound calculation.
	template<usize D>
	struct Simplex {
		static_assert(Type::Ex::Collision::GJK::Dimension<D>, "GJK only works for 2D & 3D collision!");

		/// @brief Dimension of the simplex.
		constexpr static usize DIMENSION	= D;
		/// @brief Maximum amount of points in the simplex.
		constexpr static usize MAX_POINTS	= DIMENSION+1;

		/// @brief Vector type.
		using VectorType = Vector<DIMENSION>;
		/// @brief Point array type.
		using PointArrayType = Array<VectorType, MAX_POINTS>;
		//using VectorType = Vector<DIMENSION>;

		/// @brief Default constructor.
		constexpr Simplex(): count(0) {}

		/// @brief Constructs the simplex from a list of points.
		/// @param ps Points to construct from.
		constexpr explicit Simplex(List<VectorType> const& ps): Simplex() {
			for (;count < ps.size() && count < MAX_POINTS; ++count)
				points[count] = ps[count];
		}

		/// @brief Constructs the simplex from a static array of points.
		/// @param points Points to construct from.
		template<usize S>
		constexpr Simplex(Array<VectorType, S> const& points)
		requires (S <= MAX_POINTS): points(points), count(S) {}
		
		/// @brief Returns an iterator to the beginning of the point list.
		/// @return iterator to beginning of point list.
		constexpr Iterator<VectorType const>	begin() const	{return points.begin();							}
		/// @brief Returns an iterator to the end of the point list.
		/// @return iterator to end of point list.
		constexpr Iterator<VectorType const>	end() const		{return points.end() - (MAX_POINTS - count);	}
		/// @brief Returns a pointer to the underlying point list.
		/// @return Pointer to underlying point list.
		constexpr ref<VectorType const>			data() const	{return points.data();							}
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
			if constexpr (DIMENSION == 3)	points = {vec, points[0], points[1], points[2]};
			else							points = {vec, points[0], points[1]};
			if (count < MAX_POINTS)
				++count;
			return *this;
		}

		/// @brief Remakes the simplex as the next simplex to check.
		/// @param direction Direction to remake simplex for.
		/// @return Whether simplex contains the origin.
		constexpr bool remake(VectorType& direction) requires (DIMENSION == 3) {
			switch (count) {
				case 2: return line(direction);
				case 3: return triangle(direction);
				case 4: return tetrahedron(direction);
			}
			return false;
		}

		/// @brief Remakes the simplex as the next simplex to check.
		/// @param direction Direction to remake simplex for.
		/// @return Whether simplex contains the origin.
		constexpr bool remake(VectorType& direction) requires (DIMENSION == 2) {
			switch (count) {
				case 2: return line(direction);
				case 3: return triangle(direction);
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
		usize			count	= 0;

		constexpr bool line(VectorType& direction) {
			VectorType a = points[0];
			VectorType b = points[1];
			VectorType ab = b - a;
			VectorType ao =   - a;
			if (same(ab, ao))
				direction = ab.itri(ao, ab);
			else {
				points = {a};
				direction = ao;
			}
			return false;
		}

		constexpr bool triangle(VectorType& direction) {
			VectorType a = points[0];
			VectorType b = points[1];
			VectorType c = points[2];
			VectorType ab = b - a;
			VectorType ac = c - a;
			VectorType ao =   - a;
			VectorType abc = ab.fcross(ac);
			if (same(abc.fcross(ac), ao)) {
				if (same(ac, ao)) {
					points = {a, c};
					direction = ac.itri(ao, ac);
				} else {
					points = {a, b};
					return line(direction);
				}
			} else if (same(ab.fcross(abc), ao)) {
				points = {a, b};
				return line(direction);
			} else if (same(abc, ao)) {
				direction = abc;
			} else {
				points = {a, c, b};
				direction = -abc;
			}
			return DIMENSION == 2;
		}

		constexpr bool tetrahedron(VectorType& direction) requires (DIMENSION > 2) {
			VectorType a = points[0];
			VectorType b = points[1];
			VectorType c = points[2];
			VectorType d = points[3];
			VectorType ab = b - a;
			VectorType ac = c - a;
			VectorType ad = d - a;
			VectorType ao =   - a;
			VectorType abc = ab.fcross(ac);
			VectorType acd = ac.fcross(ad);
			VectorType adb = ad.fcross(ab);
			if (same(abc, ao)) {
				points = {a, b, c};
				return triangle(direction);
			}
			if (same(acd, ao)) {
				points = {a, c, d};
				return triangle(direction);
			}
			if (same(adb, ao)) {
				points = {a, d, b};
				return triangle(direction);
			}
			return DIMENSION == 3;
		}
	};

	/// @brief Gets the support vector between two bounds.
	/// @tparam DA Dimension of bound A.
	/// @tparam DB Dimension of bound B.
	/// @param a GJK-enabled bound.
	/// @param b GJK-enabled bound.
	/// @param direction Direction to get support vector for.
	/// @return Support vector.
	template<usize DA, usize DB>
	constexpr Vector3 support(
		IGJKBound<DA> const& a,
		IGJKBound<DB> const& b,
		Vector<(DA > DB ? DA : DB)> const& direction
	) {
		return a.furthest(direction) - b.furthest(-direction);
	}

	/// @brief Checks collision between two bounds.
	/// @tparam DA Dimension of collider A.
	/// @tparam DB Dimension of collider B.
	/// @param a GJK-enabled bound.
	/// @param b GJK-enabled bound.
	/// @return Whether they collide.
	template<usize DA, usize DB>
	constexpr bool check(
		IGJKBound<DA> const& a,
		IGJKBound<DB> const& b
	) requires (Type::Ex::Collision::GJK::Dimensions<DA, DB>) {
		constexpr usize DIMENSION = (DA > DB ? DA : DB);
		if (!a.bounded(b))	return false;
		if (a.match(b))		return true;
		using VectorType = Vector<DIMENSION>;
		VectorType sup = support(a, b, VectorType::RIGHT());
		Simplex<DIMENSION> sp;
		sp.pushFront(sup);
		VectorType d = -sup;
		while (true) {
			sup = support(a, b, d);
			if (sup.dot(d) <= 0)
				return false;
			sp.pushFront(sup);
			if (sp.remake(d))
				return true;
		}
		return true;
	}
}

/// @brief Collision bound interface.
/// @tparam D Dimension.
template<usize D>
using IBound = GJK::IGJKBound<D>;

}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_GJK_H