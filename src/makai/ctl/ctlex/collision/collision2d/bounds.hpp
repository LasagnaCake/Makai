#ifndef CTL_EX_COLLISION_COLLISION2D_BOUNDS_H
#define CTL_EX_COLLISION_COLLISION2D_BOUNDS_H

#include "../../../ctl/ctl.hpp"
#include "../../math/vector.hpp"
#include "../../math/matrix.hpp"
#include "../gjk.hpp"

// TODO: Update system to use GJK & IBound2D

CTL_EX_NAMESPACE_BEGIN

/// @brief Collision facilities.
namespace Collision {}

/// @brief Two-dimensional collision.
namespace Collision::C2D {
	template<usize UUID> struct Bounded;
}

/// @brief Two-dimensional collision type constraints.
namespace Type::Ex::Collision::C2D {
	/// @brief Type must be a valid collision bound.
	template<class T>
	concept Collidable = CTL::Type::Subclass<T, CTL::Ex::Collision::C2D::Bounded<T::ID>>;
}

/// @brief Two-dimensional collision.
namespace Collision::C2D {
	namespace {
		using
			Math::center,
			Math::Transform2D,
			Math::Vector2
		;
	}

	/// @brief Basic 2D bound interface.
	using IBound2D = IBound<2>;

	/// @brief Point bound.
	struct Point: IBound2D {
		/// @brief Constructs a point bound.
		/// @param position Position.
		constexpr Point(Vector2 const& position): position(position) {}

		/// @brief Destructor.
		constexpr virtual ~Point() {}

		/// @brief Copy constructor (defaulted).
		constexpr Point(Point const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr Point(Point&& other)		= default;

		/// @brief Returns the furthest point in a given direction.
		/// @param direction Direction to get furthest point.
		/// @returns Furthest point.
		constexpr Vector2 furthest(Vector2 const& direction) const final {
			return position;
		}

		/// @brief Point position.
		Vector2 position;
	};

	/// @brief Box bound.
	struct Box: IBound2D {
		/// @brief Constructs a box bound.
		/// @param position Position.
		/// @param size Size.
		constexpr Box(
			Vector2 const& position,
			Vector2 const& size
		):
			position(position),
			size(size) {}

		/// @brief Destructor.
		constexpr virtual ~Box() {}

		/// @brief Copy constructor (defaulted).
		constexpr Box(Box const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr Box(Box&& other)		= default;
		
		/// @brief Returns the top-left corner of the box.
		/// @return Top-left corner.
		constexpr Vector2 min() const {return position - size;}
		/// @brief Returns the bottom-right corner of the box.
		/// @return Bottom-right corner.
		constexpr Vector2 max() const {return position + size;}

		/// @brief Returns the furthest point in a given direction.
		/// @param direction Direction to get furthest point.
		/// @returns Furthest point.
		constexpr Vector2 furthest(Vector2 const& direction) const final {
			Vector2 points[4] = {
				Vector2(position.x + size.x, position.y + size.y),
				Vector2(position.x + size.x, position.y - size.y),
				Vector2(position.x - size.x, position.y - size.y),
				Vector2(position.x - size.x, position.y + size.y)
			};
			Vector2 maxPoint;
			float maxDistance = CTL::NumberLimit<float>::LOWEST;
			for (Vector2 const& vertex: points) {
				float distance = vertex.dot(direction);
				if (distance > maxDistance) {
					maxDistance = distance;
					maxPoint = vertex;
				}
			}
			return maxPoint;
		}

		/// @brief Box position.
		Vector2 position;
		/// @brief Box size.
		Vector2 size;
	};

	/// @brief "Circle" bound.
	/// @details
	///		Not truly a circle - actually an ellipse.
	///
	///		Though, a circle is also technically an ellipse with equal major and minor axes.
	struct Circle: IBound2D {
		/// @brief Constructs a "circle" bound.
		/// @param position Position.
		/// @param radius Radius. By default, it is 1.
		/// @param rotation Rotation. By default, it is 0.
		constexpr Circle(
			Vector2 const& position,
			Vector2 const& radius = 1,
			float rotation = 0
		):
			position(position),
			radius(radius),
			rotation(rotation) {}
		
		/// @brief Destructor.
		constexpr virtual ~Circle() {}

		/// @brief Copy constructor (defaulted).
		constexpr Circle(Circle const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr Circle(Circle&& other)		= default;

		/// @brief Returns the circle's radius at a given angle.
		/// @param angle Angle to get the radius for.
		/// @return Radius at the given angle.
		constexpr float radiusAt(float const angle) const {
			float as, ac;
			CTL::Math::absincos(angle + rotation, as, ac);
			return (as * radius.x) + (ac * radius.y);
		}

		/// @brief Returns the furthest point in a given direction.
		/// @param direction Direction to get furthest point.
		/// @returns Furthest point.
		constexpr Vector2 furthest(Vector2 const& direction) const final {
			//return position + Math::angleV2(rotation + direction.angle()) * radius;
			return position + Math::rotateV2(direction, rotation) * radius;
		}

		/// @brief Circle position.
		Vector2 position;
		/// @brief Circle radius.
		Vector2 radius = 1;
		/// @brief Circle rotation.
		float rotation = 0;
	};

	/// @brief "Capsule" bound.
	/// @details
	///		This one is a bit complex.
	///
	///		This is a "stadium-like" ("2D-capsule-like") shape.
	///	
	///		It would be best described as the "convex hull between two equivalent ellipses".
	///
	///		Or, a rice grain.
	/// @note Based off of https://en.wikipedia.org/wiki/Stadium_(geometry)
	struct Capsule: IBound2D {
		/// @brief Constructs a capsule bound.
		/// @param position Position.
		/// @param width Width. By default, it is 1.
		/// @param length Length. By default, it is 0.
		/// @param rotation Rotation. By default, it is 0.
		constexpr Capsule(
			Vector2 const& position,
			Vector2 const& width = 1,
			float const length = 0,
			float const rotation = 0
		):
			position(position),
			width(width),
			length(length),
			rotation(rotation)	{}

		/// @brief Destructor.
		constexpr virtual ~Capsule() {}

		/// @brief Copy constructor (defaulted).
		constexpr Capsule(Capsule const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr Capsule(Capsule&& other)		= default;

		/// @brief Returns the furthest point in a given direction.
		/// @param direction Direction to get furthest point.
		/// @returns Furthest point.
		constexpr Vector2 furthest(Vector2 const& direction) const final {
			// Based off of: http://gamedev.net/forums/topic/708675-support-function-for-capsule-gjk-and-mpr/5434478/
			Vector2 const end = Math::angleV2(rotation);
			float const alignment = end.dot(direction);
			Vector2 point = width * Math::rotateV2(direction, rotation) + position;
			if (alignment > 0) point += end * length;
			return point;
		}

		/// @brief Capsule position.
		Vector2 position;
		/// @brief Capsule width.
		Vector2 width = 1;
		/// @brief Capsule length.
		float length = 1;
		/// @brief Capsule rotation.
		float rotation = 0;
	};

	/// @brief Raycast bound.
	struct Ray: IBound2D {
		/// @brief Constructs a raycast bound.
		/// @param position Position.
		/// @param length Length.
		/// @param angle Angle.
		constexpr Ray(
			Vector2 const& position,
			Vector2 const& direction
		):
			position(position),
			direction(direction) {}

		/// @brief Destructor.
		constexpr virtual ~Ray() {}

		/// @brief Copy constructor (defaulted).
		constexpr Ray(Ray const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr Ray(Ray&& other)		= default;

		/// @brief Gets a point in the line at a given distance.
		/// @param distance Distance to line origin.
		/// @return Point at distance.
		constexpr Vector2 pointAt(float const distance) const {
			return direction.normalized() * distance;
		}

		/// @brief Returns the furthest point in a given direction.
		/// @param direction Direction to get furthest point.
		/// @returns Furthest point.
		constexpr Vector2 furthest(Vector2 const& dir) const final {
			if (dir.dot(direction) <= 0)
				return position;
			return position + direction;
		}

		/// @brief Ray position.
		Vector2 position;
		/// @brief Ray direction.
		Vector2 direction;
	};

	/// @brief Convex shape bound with dynamic vertex count.
	struct Shape: IBound2D {
		/// @brief Empty constructor.
		constexpr Shape() {}

		/// @brief Destructor.
		constexpr virtual ~Shape() {}

		/// @brief Allocates space for the shape's vertices.
		/// @param size Vertex count.
		constexpr Shape(usize const size): points(size) {}

		/// @brief Constructs the shape from an array of points.
		/// @tparam S Array size.
		/// @param trans Shape transform.
		/// @param points Vertices.
		template<usize S>
		constexpr Shape(Decay::AsType<Vector2[S]> const& points): points(points)	{}

		/// @brief Constructs the shape from a set of points.
		/// @param trans Shape transform.
		/// @param points Vertices.
		constexpr Shape(Span<Vector2> const& points): points(points)				{}

		/// @brief Copy constructor (defaulted).
		constexpr Shape(Shape const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr Shape(Shape&& other)		= default;

		/// @brief Returns the furthest point in a given direction.
		/// @param direction Direction to get furthest point.
		/// @returns Furthest point.
		constexpr Vector2 furthest(Vector2 const& direction) const final {
			Vector2  maxPoint;
			float maxDistance = CTL::NumberLimit<float>::LOWEST;
			Math::Matrix3x3 mat = trans;
			for (Vector2 const& vertex: points) {
				Vector2 const tp = mat * Math::Vector3(vertex, 1); 
				float distance = tp.dot(direction);
				if (distance > maxDistance) {
					maxDistance = distance;
					maxPoint = vertex;
				}
			}
			return maxPoint;
		}
		
		/// @brief Shape transform.
		Transform2D		trans;
		/// @brief Shape vertices.
		List<Vector2>	points;
	};
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_COLLISION_COLLISION2D_BOUNDS_H