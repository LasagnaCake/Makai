#ifndef CTL_EX_MATH_VECTOR_H
#define CTL_EX_MATH_VECTOR_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/math/core.hpp"
#include "../../ctl/order.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

CTL_EX_NAMESPACE_BEGIN

namespace Math {
	class Vector2;
	class Vector3;
	class Vector4;
}

/// @brief Vector-specific type constraints.
namespace Type::Ex::Math::Vector {
	/// @brief Type must be a valid 2D vector.
	template<typename T>
	concept Vector2 = requires {
		requires	CTL::Type::Equal<T, ::CTL::Ex::Math::Vector2>;
	};

	/// @brief Type must be a valid 3D vector.
	template<typename T>
	concept Vector3 = requires {
		requires	CTL::Type::Equal<T, ::CTL::Ex::Math::Vector3>;
	};

	/// @brief Type must be a valid 4D vector.
	template<typename T>
	concept Vector4 = requires {
		requires	CTL::Type::Equal<T, ::CTL::Ex::Math::Vector4>;
	};

	/// @brief Type must be a valid vector.
	template<typename T>
	concept Vector =
		Vector2<T>
	||	Vector3<T>
	||	Vector4<T>
	;

	/// @brief Type must be a vector of some kind.
	template<typename T>
	concept Vectorable =
		CTL::Type::Number<T>
	||	CTL::Type::OneOf<
			T,
			::CTL::Ex::Math::Vector2,
			::CTL::Ex::Math::Vector3,
			::CTL::Ex::Math::Vector4
	>;

	/// @brief Types must form a valid vector operation.
	template<class A, class B>
	concept Operatable =
		Vectorable<A>
	&&	Vectorable<B>
	&&	(
			(Equal<A, B> && !Type::Number<A>)
		||	Different<A, B>
	);
}

/// @brief Math extensions.
namespace Math {

/*
	If you're wondering why operators are implemented like this,
	I was too lazy to write documentation for each specialization.
	(Also because otherwise the line count would be bigger :/)

	But hey, at least it works!
	Don't stare at it for too long, though. You'll start seeing
	things that do not exist.
*/

/// @brief Math template metaprogramming facilities.
namespace Meta {
	/// @brief Decays to a valid vector type.
	/// @tparam A Possible type.
	/// @tparam B Possible type.
	template<
		Type::Ex::Math::Vector::Vectorable A,
		Type::Ex::Math::Vector::Operatable<A> B
	>
	using VectorType = CTL::Meta::DualType<Type::Number<B>, A, B>;

	/// @brief Decays to the vector type with the most components.
	/// @tparam A Possible type.
	/// @tparam B Possible type.
	template<
		Type::Ex::Math::Vector::Vectorable A,
		Type::Ex::Math::Vector::Vectorable B
	>
	using LargestVectorType = CTL::Meta::DualType<
		sizeof(A) < sizeof(B),
		VectorType<A, B>,
		VectorType<B, A>
	>;
}

/// @brief Universal vector unary plus operator.
/// @tparam A Vector type.
/// @param v Vector to operate on.
/// @return Result of the operation.
template<Type::Ex::Math::Vector::Vector T>
constexpr T operator+(T const& v) {
	if constexpr (Type::Equal<T, Vector2>)
		return T(v.x, v.y);
	else if constexpr (Type::Equal<T, Vector3>)
		return T(v.x, v.y, v.z);
	else if constexpr (Type::Equal<T, Vector4>)
		return T(v.x, v.y, v.z, v.w);
}

/// @brief Universal vector unary minus operator.
/// @tparam A Vector type.
/// @param v Vector to operate on.
/// @return Result of the operation.
template<Type::Ex::Math::Vector::Vector T>
constexpr T operator-(T const& v) {
	if constexpr (Type::Equal<T, Vector2>)
		return T(-v.x, -v.y);
	else if constexpr (Type::Equal<T, Vector3>)
		return T(-v.x, -v.y, -v.z);
	else if constexpr (Type::Equal<T, Vector4>)
		return T(-v.x, -v.y, -v.z, -v.w);
}


/// @brief Universal vector addition operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vectorable A,
	Type::Ex::Math::Vector::Operatable<A> B
>
constexpr Meta::VectorType<A, B> operator+(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestVectorType<A, B>(a) + Meta::LargestVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, Vector2>)
		return A(a.x + b.x, a.y + b.y);
	else if constexpr (Type::Equal<A, Vector3>)
		return A(a.x + b.x, a.y + b.y, a.z + b.z);
	else if constexpr (Type::Equal<A, Vector4>)
		return A(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

/// @brief Universal vector subtraction operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vectorable A,
	Type::Ex::Math::Vector::Operatable<A> B
>
constexpr Meta::VectorType<A, B> operator-(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestVectorType<A, B>(a) - Meta::LargestVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, Vector2>)
		return A(a.x - b.x, a.y - b.y);
	else if constexpr (Type::Equal<A, Vector3>)
		return A(a.x - b.x, a.y - b.y, a.z - b.z);
	else if constexpr (Type::Equal<A, Vector4>)
		return A(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

/// @brief Universal vector multiplication operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vectorable A,
	Type::Ex::Math::Vector::Operatable<A> B
>
constexpr Meta::VectorType<A, B> operator*(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestVectorType<A, B>(a) * Meta::LargestVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, Vector2>)
		return A(a.x * b.x, a.y * b.y);
	else if constexpr (Type::Equal<A, Vector3>)
		return A(a.x * b.x, a.y * b.y, a.z * b.z);
	else if constexpr (Type::Equal<A, Vector4>)
		return A(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

/// @brief Universal vector division operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vectorable A,
	Type::Ex::Math::Vector::Operatable<A> B
>
constexpr Meta::VectorType<A, B> operator/(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestVectorType<A, B>(a) / Meta::LargestVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, Vector2>)
		return A(a.x / b.x, a.y / b.y);
	else if constexpr (Type::Equal<A, Vector3>)
		return A(a.x / b.x, a.y / b.y, a.z / b.z);
	else if constexpr (Type::Equal<A, Vector4>)
		return A(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

/// @brief Universal vector modulo operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vectorable A,
	Type::Ex::Math::Vector::Operatable<A> B
>
constexpr Meta::VectorType<A, B> operator%(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestVectorType<A, B>(a) % Meta::LargestVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, Vector2>)
		return A(fmod(a.x, b.x), fmod(a.y, b.y));
	else if constexpr (Type::Equal<A, Vector3>)
		return A(fmod(a.x, b.x), fmod(a.y, b.y), fmod(a.z, b.z));
	else if constexpr (Type::Equal<A, Vector4>)
		return A(fmod(a.x, b.x), fmod(a.y, b.y), fmod(a.z, b.z), fmod(a.w, b.w));
}

/// @brief Universal vector exponentiation operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vectorable A,
	Type::Ex::Math::Vector::Operatable<A> B
>
constexpr Meta::VectorType<A, B> operator^(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestVectorType<A, B>(a) ^ Meta::LargestVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, Vector2>)
		return A(pow(a.x, b.x), pow(a.y, b.y));
	else if constexpr (Type::Equal<A, Vector3>)
		return A(pow(a.x, b.x), pow(a.y, b.y), pow(a.z, b.z));
	else if constexpr (Type::Equal<A, Vector4>)
		return A(pow(a.x, b.x), pow(a.y, b.y), pow(a.z, b.z), pow(a.w, b.w));
}

/// @brief Universal vector addition assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vector A,
	Type::Ex::Math::Vector::Vectorable B
>
constexpr Meta::VectorType<A, B> operator+=(A& a, B const& b) {
	return a = a + b;
}

/// @brief Universal vector subtraction assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vector A,
	Type::Ex::Math::Vector::Vectorable B
>
constexpr Meta::VectorType<A, B> operator-=(A& a, B const& b) {
	return a = a - b;
}

/// @brief Universal vector multiplication assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vector A,
	Type::Ex::Math::Vector::Vectorable B
>
constexpr Meta::VectorType<A, B> operator*=(A& a, B const& b) {
	return a = a * b;
}

/// @brief Universal vector division assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vector A,
	Type::Ex::Math::Vector::Vectorable B
>
constexpr Meta::VectorType<A, B> operator/=(A& a, B const& b) {
	return a = a / b;
}

/// @brief Universal vector modulo assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vector A,
	Type::Ex::Math::Vector::Vectorable B
>
constexpr Meta::VectorType<A, B> operator%=(A& a, B const& b) {
	return a = a % b;
}

/// @brief Universal vector exponentiation assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vector A,
	Type::Ex::Math::Vector::Vectorable B
>
constexpr Meta::VectorType<A, B> operator^=(A& a, B const& b) {
	return a = a ^ b;
}

/// @brief Universal vector comparison operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vectorable A,
	Type::Ex::Math::Vector::Operatable<A> B
>
constexpr bool operator==(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestVectorType<A, B>(a) == Meta::LargestVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, Vector2>)
		return a.x == b.x && a.y == b.y;
	else if constexpr (Type::Equal<A, Vector3>)
		return a.x == b.x && a.y == b.y && a.z == b.z;
	else if constexpr (Type::Equal<A, Vector4>)
		return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

/// @brief Universal vector threeway comparison operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::Vector::Vectorable A,
	Type::Ex::Math::Vector::Operatable<A> B
>
constexpr Meta::VectorType<A, B>::OrderType operator<=>(A const& a, B const& b) {
	using Order = Meta::VectorType<A, B>::Order;
	if constexpr (Type::Different<A, B>)
		return Meta::LargestVectorType<A, B>(a) <=> Meta::LargestVectorType<A, B>(b);
	typename Meta::VectorType<A, B>::OrderType	ord = a.x <=> b.x;
	if constexpr (Type::Equal<A, Vector2>)
		if (ord == Order::EQUAL)				ord = a.y <=> b.y;
	if constexpr (Type::Equal<A, Vector3>)
		if (ord == Order::EQUAL)				ord = a.z <=> b.z;
	if constexpr (Type::Equal<A, Vector4>)
		if (ord == Order::EQUAL)				ord = a.w <=> b.w;
	return ord;
}

#pragma pack(push, 1)
/// @brief Two-dimensional vector.
class Vector2: Ordered {
	public:
		using Ordered::OrderType, Ordered::Order;

		/// @brief Vector components.
		union {
			struct {float x, y;		};
			struct {float nx, ny;	};
			struct {float u, v;		};
			float data[2] = {0};
		};

		/// @brief Default constructor.
		constexpr Vector2() {}

		/// @brief Constructs the vector's components with a starting value.
		/// @param v Value to set.
		constexpr Vector2(float const v):
			x(v), y(v) {}

		/// @brief Constructs the vector's components with a set of values.
		/// @param x X component.
		/// @param y Y component.
		constexpr Vector2(float const x, float const y):
			x(x), y(y) {}

		/// @brief Copy constructor.
		/// @tparam T Vector type.
		/// @param vec Vector to copy from.
		template<Type::OneOf<Vector2, Vector3, Vector4> T>
		constexpr Vector2(T const& vec): Vector2(vec.x, vec.y) {}

		/// @brief Constructs the vector from an array of values.
		/// @param data Values to construct from.
		constexpr Vector2(As<float[2]> const& data):
			Vector2(data[0], data[1]) {}

		/// @brief Destructor.
		constexpr ~Vector2() {}

		/// @brief Returns a "all-zeroes" vector.
		/// @return "All-zeroes" vector.
		constexpr static Vector2 ZERO()		{return 0;					}
		/// @brief Returns an "all-ones" vector.
		/// @return "All-ones" vector.
		constexpr static Vector2 ONE()		{return 1;					}
		/// @brief Returns a vector pointing towards the positive X axis.
		/// @return Vector pointing towards +X.
		constexpr static Vector2 RIGHT()	{return Vector2(+1, +0);	}
		/// @brief Returns a vector pointing towards the negative X axis.
		/// @return Vector pointing towards -X.
		constexpr static Vector2 LEFT()		{return Vector2(-1, +0);	}
		/// @brief Returns a vector pointing towards the positive Y axis.
		/// @return Vector pointing towards +Y.
		constexpr static Vector2 UP()		{return Vector2(+0, +1);	}
		/// @brief Returns a vector pointing towards the negative Y axis.
		/// @return Vector pointing towards -Y.
		constexpr static Vector2 DOWN()		{return Vector2(+0, -1);	}

		/// @brief Gets a component at a given index.
		/// @param pos Index of the component.
		/// @return Reference to component.
		constexpr float& operator[](usize const pos)		{if (pos > 1) return data[0]; return data[pos];}
		/// @brief Gets a component at a given index.
		/// @param pos Index of the component.
		/// @return Value of the component.
		constexpr float operator[](usize const pos) const	{if (pos > 1) return data[0]; return data[pos];}

		/// @brief Gets the sum of the vector's components.
		/// @returns Sum of components.
		constexpr float sum() const {
			return x + y;
		}

		/// @brief Gets the average of the vector's components.
		/// @returns Average of components.
		constexpr float average() const {
			return sum() / 2;
		}

		/// @brief Gets the smallest of the vector's components.
		/// @returns Smallest of component.
		constexpr float min() const {
			return (x < y) ? x : y;
		}

		/// @brief Gets the biggest of the vector's components.
		/// @returns Biggest component.
		constexpr float max() const {
			return (x > y) ? x : y;
		}

		/// @brief Returns a vector containing the smallest components between it and another vector.
		/// @param other Other vector.
		/// @return Smallest vector.
		constexpr Vector2 min(Vector2 const& other) const {
			return Vector2(
				CTL::Math::min(x, other.x),
				CTL::Math::min(y, other.y)
			);
		}

		/// @brief Returns a vector containing the biggest components between it and another vector.
		/// @param other Other vector.
		/// @return Biggest vector.
		constexpr Vector2 max(Vector2 const& other) const {
			return Vector2(
				CTL::Math::max(x, other.x),
				CTL::Math::max(y, other.y)
			);
		}

		/// @brief Gets the vector's length.
		/// @brief Length of vector.
		constexpr float length() const {
			return CTL::Math::sqrt((x * x) + (y * y));
		}

		/// @brief Gets the vector's squared length.
		/// @brief Squared length of vector.
		constexpr float lengthSquared() const {
			return ((x * x) + (y * y));
		}

		/// @brief Gets the normalized vector.
		/// @returns Normalized vector.
		constexpr Vector2 normalized() const {
			if (*this != 0)
				return *this / length();
			else
				return *this;
		}

		/// @brief Normalizes the vector.
		/// @returns Reference to self.
		constexpr Vector2& normalize() {
			return *this = normalized();
		}

		/// @brief Gets the distance to another vector.
		/// @param vec Vector to get distance to.
		/// @return Distance between vectors.
		constexpr float distanceTo(Vector2 const& vec) const {
			Vector2 diff = vec - *this;
			return diff.length();
		}

		/// @brief Gets the squared distance to another vector.
		/// @param vec Vector to get distance to.
		/// @return Distance between vectors.
		constexpr float squaredDistanceTo(Vector2 const& vec) const {
			Vector2 diff = vec - *this;
			return diff.lengthSquared();
		}

		/// @brief Gets the vector's angle.
		/// @return Angle of vector.
		constexpr float angle() const {
			return CTL::Math::atan2(x, y);
		}

		/// @brief Gets the vector's angle to another vector.
		/// @param vec Vector to get angle to.
		/// @return Angle to vector.
		constexpr float angleTo(Vector2 const& vec) const {
			Vector2 diff = vec - *this;
			return diff.angle();
		}

		/// @brief Gets a normal pointing towards another vector.
		/// @param vec Vector to get normal to.
		/// @return Normal to vector.
		constexpr Vector2 normalTo(Vector2 const& vec) const {
			Vector2 diff = vec - *this;
			return diff.normalized();
		}


		/// @brief Clamps the vector between two values.
		/// @param min Minimum.
		/// @param max Maximum.
		/// @return Reference to self.
		constexpr Vector2& clamp(Vector2 const& min, Vector2 const& max) {
			x = ::CTL::Math::clamp(x, min.x, max.x);
			y = ::CTL::Math::clamp(y, min.y, max.y);
			return *this;
		}

		/// @brief Returns the vector clamped between two values.
		/// @param min Minimum.
		/// @param max Maximum.
		/// @return Clamped vector.
		constexpr Vector2 clamped(Vector2 const& min, Vector2 const& max) const {
			return Vector2(
				::CTL::Math::clamp(x, min.x, max.x),
				::CTL::Math::clamp(y, min.y, max.y)
			);
		}

		/// @brief Gets the tangent of the vector.
		/// @return Tangent of vector.
		constexpr float tangent() const {
			return x / y;
		}

		/// @brief Returns the dot product with another vector.
		/// @param vec Vector to get dot product with.
		/// @return Dot product between vectors.
		constexpr float dot(Vector2 const& vec) const {
			Vector2 mult = (*this) * vec;
			return mult.x + mult.y;
		}

		/// @brief Returns the cross product with another vector.
		/// @param vec Vector to get cross product with.
		/// @return Cross product between vectors.
		constexpr float cross(Vector2 const& vec) const {
			return (x * vec.y) - (y * vec.x);
		}

		/// @brief Returns the "cross product" with another vector via the triple product.
		/// @param vec Vector to get cross product with.
		/// @return "Cross product" between vectors.
		constexpr Vector2 fcross(Vector2 const& vec) const {
			return tri(vec, vec);
		}

		/// @brief Returns the triple cross product `A x (B x C)` with two other vectors.
		/// @param b Vector to get triple cross product with.
		/// @param c Vector to get triple cross product with.
		/// @return Triple cross product.
		constexpr Vector2 tri(Vector2 const& b, Vector2 const& c) const {
			return (b * dot(c)) - (c * dot(b));
		}

		/// @brief Returns the inverse triple cross product `(A x B) x C` with two other vectors.
		/// @param b Vector to get triple cross product with.
		/// @param c Vector to get triple cross product with.
		/// @return Triple cross product.
		constexpr Vector2 itri(Vector2 const& b, Vector2 const& c) const {
			return - c.tri(*this, b);
		}

		/// @brief Returns this vector projected in another vector.
		/// @param vec Vector to project in.
		/// @return Result of the operation.
		constexpr Vector2 projected(Vector2 const& vec) const {
			return (vec.dot(*this) / vec.dot(vec)) * vec;
		}

		/// @brief Projects this vector in another vector.
		/// @param vec Vector to project in.
		/// @return Reference to self.
		constexpr Vector2& project(Vector2 const& vec) {
			return *this = projected(vec);
		}

		/// @brief Returns the absolute value of the vector.
		/// @return Absolute vector.
		constexpr Vector2 absolute() const {
			return Vector2(
				CTL::Math::abs(x),
				CTL::Math::abs(y)
			);
		}

		/// @brief Returns the vector in (Y, X) form.
		/// @return Vector in (Y, X) form.
		constexpr Vector2 yx() const {
			return Vector2(y, x);
		}
};

/// @brief Three-dimensional vector.
class Vector3: Ordered {
	public:
		using Ordered::OrderType, Ordered::Order;

		/// @brief Vector components.
		union {
			struct {float x, y, z;		};
			struct {float nx, ny, nz;	};
			struct {float r, g, b;		};
			struct {float u, v, t;		};
			float data[3] = {0};
		};

		/// @brief Default constructor.
		constexpr Vector3() {}

		/// @brief Constructs the vector's components with a starting value.
		/// @param v Value to set.
		constexpr Vector3(float v):
			x(v), y(v), z(v) {}

		/// @brief Constructs the vector's components from a set of values.
		/// @param x X component.
		/// @param y Y component.
		/// @param z Z component. By default, it is 0.
		constexpr Vector3(float const x, float const y, float const z = 0.0):
			x(x), y(y), z(z) {}

		/// @brief Copy constructor.
		/// @tparam T Vector type.
		/// @param vec Vector to copy from.
		template<Type::OneOf<Vector3, Vector4> T>
		constexpr Vector3(T const& vec):
			Vector3(vec.x, vec.y, vec.z) {}

		/// @brief Constructs the vector's components from a vector and a value.
		/// @param vec Vector to use for X and Y components.
		/// @param z Z component. By default, it is 0.
		constexpr Vector3(Vector2 const& vec, float z = 0):
			Vector3(vec.x, vec.y, z) {}

		/// @brief Constructs the vector from an array of values.
		/// @param data Values to construct from.
		constexpr Vector3(As<float[3]> const& data):
			Vector3(data[0], data[1], data[2]) {}

		/// @brief Destructor.
		constexpr ~Vector3() {}

		/// @brief Returns an "all-zeroes" vector.
		/// @return "All-zeroes" vector.
		constexpr static Vector3 ZERO()		{return 0;						}
		/// @brief Returns an "all-ones" vector.
		/// @return "All-ones" vector.
		constexpr static Vector3 ONE()		{return 1;						}
		/// @brief Returns a vector pointing towards the positive X axis.
		/// @return Vector pointing towards +x.
		constexpr static Vector3 RIGHT()	{return Vector2::RIGHT();		}
		/// @brief Returns a vector pointing towards the negative X axis.
		/// @return Vector pointing towards -x.
		constexpr static Vector3 LEFT()		{return Vector2::LEFT();		}
		/// @brief Returns a vector pointing towards the positive Y axis.
		/// @return Vector pointing towards +Y.
		constexpr static Vector3 UP()		{return Vector2::UP();			}
		/// @brief Returns a vector pointing towards the negative Y axis.
		/// @return Vector pointing towards -Y.
		constexpr static Vector3 DOWN()		{return Vector2::DOWN();		}
		/// @brief Returns a vector pointing towards the positive Z axis.
		/// @return Vector pointing towards +Z.
		constexpr static Vector3 BACK()		{return Vector3(+0, +0, +1);	}
		/// @brief Returns a vector pointing towards the negative Z axis.
		/// @return Vector pointing towards -Z.
		constexpr static Vector3 FRONT()	{return Vector3(+0, +0, -1);	}

		/// @brief Gets a component at a given index.
		/// @param pos Index of the component.
		/// @return Reference to component.
		constexpr float& operator[](usize const pos)		{if (pos > 2) return data[0]; return data[pos];}
		/// @brief Gets a component at a given index.
		/// @param pos Index of the component.
		/// @return Value of component.
		constexpr float operator[](usize const pos) const	{if (pos > 2) return data[0]; return data[pos];}

		/// @brief Gets the sum of the vector's components.
		/// @return Sum of components.
		constexpr float sum() const {
			return x + y + z;
		}

		/// @brief Gets the average of the vector's components.
		/// @return Average of components.
		constexpr float average() const {
			return sum() / 3;
		}

		/// @brief Gets the smallest of the vector's components.
		/// @return Smallest component.
		constexpr float min() const {
			return ::CTL::Math::min(::CTL::Math::min(x, y), z);
		}

		/// @brief Gets the biggest of the vector's components.
		/// @return Biggest component.
		constexpr float max() const {
			return ::CTL::Math::max(::CTL::Math::max(x, y), z);
		}

		/// @brief Returns a vector containing the smallest components between it and another vector.
		/// @param other Other vector.
		/// @return Smallest vector.
		constexpr Vector3 min(Vector3 const& other) const {
			return Vector3(
				CTL::Math::min(x, other.x),
				CTL::Math::min(y, other.y),
				CTL::Math::min(z, other.z)
			);
		}

		/// @brief Returns a vector containing the biggest components between it and another vector.
		/// @param other Other vector.
		/// @return Biggest vector.
		constexpr Vector3 max(Vector3 const& other) const {
			return Vector3(
				CTL::Math::max(x, other.x),
				CTL::Math::max(y, other.y),
				CTL::Math::min(z, other.z)
			);
		}

		/// @brief Gets the vector's length.
		/// @return Length of vector.
		constexpr float length() const {
			return CTL::Math::sqrt((x * x) + (y * y) + (z * z));
		}

		/// @brief Gets the vector's squared length.
		/// @return Squared ength of vector.
		constexpr float lengthSquared() const {
			return ((x * x) + (y * y) + (z * z));
		}

		/// @brief Gets the vector's angle.
		/// @return Angle of vector.
		constexpr Vector3 angle() const {
			Vector3 res;
			float mag = length();
			res.x = acos(x/mag);
			res.y = acos(y/mag);
			res.z = acos(z/mag);
			return res;
		}

		/// @brief Gets the vector's angle to another vector.
		/// @param vec Vector to get angle to.
		/// @return Angle to vector.
		constexpr Vector3 angleTo(Vector3 const& vec) const {
			Vector3 diff = vec - *this;
			return diff.angle();
		}

		/// @brief Gets the normalized vector.
		/// @return Normalized vector.
		constexpr Vector3 normalized() const {
			if (*this != 0)
				return *this / length();
			else
				return *this;
		}

		/// @brief Normalizes the vector.
		/// @return Reference to self.
		constexpr Vector3& normalize() {
			return *this = normalized();
		}

		/// @brief Gets a normal pointing towards another vector.
		/// @param vec Vector to get normal to.
		/// @return Normal to vector.
		constexpr Vector3 normalTo(Vector3 const& vec) const {
			Vector3 diff = vec - *this;
			return diff.normalized();
		}

		/// @brief Gets the distance to another vector.
		/// @param vec Vector to get distance to.
		/// @return Distance to vector.
		constexpr float distanceTo(Vector3 const& vec) const {
			Vector3 diff = vec - *this;
			return diff.length();
		}

		/// @brief Gets the squared distance to another vector.
		/// @param vec Vector to get squared distance to.
		/// @return Squared distance to vector.
		constexpr float squaredDistanceTo(Vector3 const& vec) const {
			Vector3 diff = vec - *this;
			return diff.lengthSquared();
		}

		/// @brief Clamps the vector between two values.
		/// @param min Minimum.
		/// @param max Maximum.
		/// @return Reference to self.
		constexpr Vector3& clamp(Vector3 const& min, Vector3 const& max) {
			x = ::CTL::Math::clamp(x, min.x, max.x);
			y = ::CTL::Math::clamp(y, min.y, max.y);
			z = ::CTL::Math::clamp(z, min.z, max.z);
			return *this;
		}

		/// @brief Returns the vector clamped between two values.
		/// @param min Minimum.
		/// @param max Maximum.
		/// @return Clamped vector.
		constexpr Vector3 clamped(Vector3 const& min, Vector3 const& max) const {
			return Vector3(
				::CTL::Math::clamp(x, min.x, max.x),
				::CTL::Math::clamp(y, min.y, max.y),
				::CTL::Math::clamp(z, min.z, max.z)
			);
		}

		/// @brief Returns the dot product with another vector.
		/// @param vec Vector to get dot product with.
		/// @return Dot product.
		constexpr float dot(Vector3 const& vec) const {
			Vector3 mult = (*this) * vec;
			return mult.x + mult.y + mult.z;
		}

		/// @brief Returns the cross product with another vector.
		/// @param vec Vector to get cross product with.
		/// @return Cross product.
		constexpr Vector3 cross(Vector3 const& vec) const {
			return Vector3(
				(y * vec.z) - (z * vec.y),
				(z * vec.x) - (x * vec.z),
				(x * vec.y) - (y * vec.x)
			);
		}

		/// @brief Returns the "cross product" with another vector via the triple product.
		/// @param vec Vector to get cross product with.
		/// @return "Cross product" between vectors.
		constexpr Vector3 fcross(Vector3 const& vec) const {
			return tri(vec, vec);
		}

		/// @brief Returns the triple cross product `A x (B x C)` with two other vectors.
		/// @param b Vector to get triple cross product with.
		/// @param c Vector to get triple cross product with.
		/// @return Triple cross product.
		constexpr Vector3 tri(Vector3 const& b, Vector3 const& c) const {
			return (b * dot(c)) - (c * dot(b));
		}

		/// @brief Returns the inverse triple cross product `(A x B) x C` with two other vectors.
		/// @param b Vector to get triple cross product with.
		/// @param c Vector to get triple cross product with.
		/// @return Triple cross product.
		constexpr Vector3 itri(Vector3 const& b, Vector3 const& c) const {
			return - c.tri(*this, b);
		}

		/// @brief Returns the mixed (scalar triple) product with two other vectors (`this` dot (`a` cross `b`)).
		/// @param a Vector to get mixed product with.
		/// @param b Vector to get mixed product with.
		/// @return Mixed product.
		constexpr float mix(Vector3 const& a, Vector3 const& b) const {
			return dot(a.cross(b));
		}

		/// @brief Returns this vector projected in another vector.
		/// @param vec Vector to project in.
		/// @return Result of the operation.
		constexpr Vector3 projected(Vector3 const& vec) const {
			return (vec.dot(*this) / vec.dot(vec)) * vec;
		}

		/// @brief Projects this vector in another vector.
		/// @param vec Vector to project in.
		/// @return Reference to self.
		constexpr Vector3& project(Vector3 const& vec) {
			return *this = projected(vec);
		}

		/// @brief Returns the X and Y components.
		/// @return X and Y components.
		constexpr Vector2 xy() const {
			return Vector2(x, y);
		}

		/// @brief Returns the Y and Z components.
		/// @return Y and Z components.
		constexpr Vector2 yz() const {
			return Vector2(x, y);
		}

		/// @brief Returns the X and Z components.
		/// @return X and Z components.
		constexpr Vector2 xz() const {
			return Vector2(x, y);
		}

		/// @brief Returns the vector in (Z, Y, X) form.
		/// @return Vector in (Z, Y, X) form.
		constexpr Vector3 zyx() const {
			return Vector3(z, y, x);
		}

		/// @brief Returns the vector in (X, Z, Y) form.
		/// @return Vector in (X, Z, Y) form.
		constexpr Vector3 xzy() const {
			return Vector3(x, z, y);
		}

		/// @brief Returns the vector in (Y, Z, X) form.
		/// @return Vector in (Y, Z, X) form.
		constexpr Vector3 yzx() const {
			return Vector3(y, z, x);
		}

		/// @brief Returns the absolute value of the vector.
		/// @return Absolute vector.
		constexpr Vector3 absolute() const {
			return Vector3(
				CTL::Math::abs(x),
				CTL::Math::abs(y),
				CTL::Math::abs(z)
			);
		}
};

/// @brief Four-dimensional vector.
class Vector4: Ordered {
	public:
		using Ordered::OrderType, Ordered::Order;

		/// The vector's position.
		union {
			struct {float x, y, z, w;		};
			struct {float nx, ny, nz, nw;	};
			struct {float r, g, b, a;		};
			struct {float u, v, t, s;		};
			float data[4] = {0};
		};

		/// @brief Default constructor.
		constexpr Vector4() {}

		/// @brief Constructs the vector's components with a starting value.
		/// @param v Value to set.
		constexpr Vector4(float v):
			x(v), y(v), z(v), w(v) {}

		/// @brief Constructs the vector's components from a set of values.
		/// @param x X component.
		/// @param y Y component.
		/// @param z Z component.
		/// @param w W component. By default, it is 0.
		constexpr Vector4(float const x, float const y, float const z, float const w = 0.0):
			x(x), y(y), z(z), w(w) {}

		/// @brief Constructs the vector's components from a set of 2D vectors.
		/// @param v1 Vector to use for X and Y components.
		/// @param v2 Vector to use for Z and W components.
		constexpr Vector4(Vector2 const& v1, Vector2 const& v2):
			Vector4(v1.x, v1.y, v2.x, v2.y) {}

		/// @brief Copy constructor.
		/// @param vec `Vector4` to copy from.
		constexpr Vector4(Vector4 const& vec):
			Vector4(vec.x, vec.y, vec.z, vec.w) {}

		/// @brief Constructs the vector's components from a vector and a value.
		/// @param vec Vector to use for X, Y and Z components.
		/// @param w W component. By default, it is 0.
		constexpr Vector4(Vector3 const& vec, float w = 0):
			Vector4(vec.x, vec.y, vec.z, w) {}

		/// @brief Constructs the vector's components from a vector and a value.
		/// @param vec Vector to use for X and Y components.
		/// @param z Z component. By default, it is 0.
		/// @param w W component. By default, it is 0.
		constexpr Vector4(Vector2 const& vec, float z = 0, float w = 0):
			Vector4(vec.x, vec.y, z, w) {}

		/// @brief Constructs the vector from an array of values.
		/// @param data Values to construct from.
		constexpr Vector4(As<float[4]> const& data):
			Vector4(data[0], data[1], data[2], data[3]) {}

		/// @brief Destructor.
		constexpr ~Vector4() {}

		/// @brief Returns an "all-zeroes" vector.
		/// @return "All-zeroes" vector.
		constexpr static Vector4 ZERO()		{return 0;							}
		/// @brief Returns an "all-ones" vector.
		/// @return "All-ones" vector.
		constexpr static Vector4 ONE()		{return 1;							}
		/// @brief Returns a vector pointing towards the positive X axis.
		/// @return vector pointing towards +X.
		constexpr static Vector4 RIGHT()	{return Vector2::RIGHT();			}
		/// @brief Returns a vector pointing towards the negative X axis.
		/// @return vector pointing towards -X.
		constexpr static Vector4 LEFT()		{return Vector2::LEFT();			}
		/// @brief Returns a vector pointing towards the positive Y axis.
		/// @return vector pointing towards +Y.
		constexpr static Vector4 UP()		{return Vector2::UP();				}
		/// @brief Returns a vector pointing towards the negative Y axis.
		/// @return vector pointing towards -Y.
		constexpr static Vector4 DOWN()		{return Vector2::DOWN();			}
		/// @brief Returns a vector pointing towards the positive Z axis.
		/// @return vector pointing towards +Z.
		constexpr static Vector4 BACK()		{return Vector3::BACK();			}
		/// @brief Returns a vector pointing towards the negative Z axis.
		/// @return vector pointing towards -Z.
		constexpr static Vector4 FRONT()	{return Vector3::FRONT();			}
		/// @brief Returns a vector pointing towards the positive W axis.
		/// @return vector pointing towards +W.
		constexpr static Vector4 FUTURE()	{return Vector4(+0, +0, +0, +1);	}
		/// @brief Returns a vector pointing towards the negative W axis.
		/// @return vector pointing towards -W.
		constexpr static Vector4 PAST()		{return Vector4(+0, +0, +0, -1);	}
		/// @brief Returns a vector pointing towards the positive W axis.
		/// @return vector pointing towards +W.
		constexpr static Vector4 ANA()		{return Vector4(+0, +0, +0, +1);	}
		/// @brief Returns a vector pointing towards the negative W axis.
		/// @return vector pointing towards -W.
		constexpr static Vector4 KATA()		{return Vector4(+0, +0, +0, -1);	}

		/// @brief Gets a component at a given index.
		/// @param pos Index of the component.
		/// @return Reference to component.
		constexpr float& operator[](usize const pos)		{if (pos > 3) return data[0]; return data[pos];}
		/// @brief Gets a component at a given index.
		/// @param pos Index of the component.
		/// @return Value of component.
		constexpr float operator[](usize const pos) const	{if (pos > 3) return data[0]; return data[pos];}

		/// @brief Gets the sum of the vector's components.
		/// @return Sum of components.
		constexpr float sum() const {
			return x + y + z + w;
		}

		/// @brief Gets the average of the vector's components.
		/// @return Average of components.
		constexpr float average() const {
			return sum() / 4;
		}

		/// @brief Gets the smallest of the vector's components.
		/// @return Smallest component.
		constexpr float min() const {
			return ::CTL::Math::min(::CTL::Math::min(::CTL::Math::min(x, y), z), w);
		}

		/// @brief Gets the biggest of the vector's components.
		/// @return Biggest component.
		constexpr float max() const {
			return ::CTL::Math::max(::CTL::Math::max(::CTL::Math::max(x, y), z), w);
		}

		/// @brief Returns a vector containing the smallest components between it and another vector.
		/// @param other Other vector.
		/// @return Smallest vector.
		constexpr Vector4 min(Vector4 const& other) const {
			return Vector4(
				CTL::Math::min(x, other.x),
				CTL::Math::min(y, other.y),
				CTL::Math::min(z, other.z),
				CTL::Math::min(w, other.w)
			);
		}

		/// @brief Returns a vector containing the biggest components between it and another vector.
		/// @param other Other vector.
		/// @return Biggest vector.
		constexpr Vector4 max(Vector4 const& other) const {
			return Vector4(
				CTL::Math::max(x, other.x),
				CTL::Math::max(y, other.y),
				CTL::Math::min(z, other.z),
				CTL::Math::min(w, other.w)
			);
		}

		/// @brief Gets the vector's length.
		/// @return Length of vector.
		constexpr float length() const {
			return CTL::Math::sqrt((x * x) + (y * y) + (z * z) + (w * w));
		}

		/// @brief Gets the vector's squared length.
		/// @return Squared length of vector.
		constexpr float lengthSquared() const {
			return ((x * x) + (y * y) + (z * z) + (w * w));
		}

		/// @brief Gets the normalized vector.
		/// @return Normalized vector.
		constexpr Vector4 normalized() const {
			if (*this != 0)
				return *this / length();
			else
				return *this;
		}

		/// @brief Normalizes the vector.
		/// @return Reference to self.
		constexpr Vector4& normalize() {
			return *this = normalized();
		}

		/// @brief Gets the distance to another vector.
		/// @param vec Vector to get distance to.
		/// @return Distance between vectors.
		constexpr float distanceTo(Vector4 const& vec) const {
			Vector4 diff = vec - *this;
			return diff.length();
		}

		/// @brief Gets the squared distance to another vector.
		/// @param vec Vector to get squared distance to.
		/// @return Squared distance between vectors.
		constexpr float squaredDistanceTo(Vector4 const& vec) const {
			Vector4 diff = vec - *this;
			return diff.lengthSquared();
		}

		/// @brief Gets a normal pointing towards another vector.
		/// @param vec Vector to get normal to.
		/// @return Normal to vector.
		constexpr Vector4 normalTo(Vector4 const& vec) const {
			Vector4 diff = vec - *this;
			return diff.normalized();
		}

		/// @brief Clamps the vector between two values.
		/// @param min Minimum.
		/// @param max Maximum.
		/// @return Reference to self.
		constexpr Vector4& clamp(Vector4 const& min, Vector4 const& max) {
			x = ::CTL::Math::clamp(x, min.x, max.x);
			y = ::CTL::Math::clamp(y, min.y, max.y);
			z = ::CTL::Math::clamp(z, min.z, max.z);
			z = ::CTL::Math::clamp(w, min.w, max.w);
			return *this;
		}

		/// @brief Returns the vector clamped between two values.
		/// @param min Minimum.
		/// @param max Maximum.
		/// @return Clamped vector.
		constexpr Vector4 clamped(Vector4 const& min, Vector4 const& max) const {
			return Vector4(
				::CTL::Math::clamp(x, min.x, max.x),
				::CTL::Math::clamp(y, min.y, max.y),
				::CTL::Math::clamp(z, min.z, max.z),
				::CTL::Math::clamp(w, min.w, max.w)
			);
		}

		/// @brief Returns the dot product with another vector.
		/// @param vec Vector to get dot product with.
		/// @return Dot product.
		constexpr float dot(Vector4 const& vec) const {
			Vector4 mult = (*this) * vec;
			return mult.x + mult.y + mult.z + mult.w;
		}

		/// @brief Returns the "cross product" with another vector via the triple product.
		/// @param vec Vector to get cross product with.
		/// @return "Cross product" between vectors.
		constexpr Vector4 fcross(Vector4 const& vec) const {
			return tri(vec, vec);
		}

		/// @brief Returns the triple cross product `A x (B x C)` with two other vectors.
		/// @param b Vector to get triple cross product with.
		/// @param c Vector to get triple cross product with.
		/// @return Triple cross product.
		constexpr Vector4 tri(Vector4 const& b, Vector4 const& c) const {
			return (b * dot(c)) - (c * dot(b));
		}

		/// @brief Returns the inverse triple cross product `(A x B) x C` with two other vectors.
		/// @param b Vector to get triple cross product with.
		/// @param c Vector to get triple cross product with.
		/// @return Triple cross product.
		constexpr Vector4 itri(Vector4 const& b, Vector4 const& c) const {
			return - c.tri(*this, b);
		}

		/// @brief Returns this vector projected in another vector.
		/// @param vec Vector to project in.
		/// @return Result of the operation.
		constexpr Vector4 projected(Vector4 const& vec) const {
			return (vec.dot(*this) / vec.dot(vec)) * vec;
		}

		/// @brief Projects this vector in another vector.
		/// @param vec Vector to project in.
		/// @return Reference to self.
		constexpr Vector4& project(Vector4 const& vec) {
			return *this = projected(vec);
		}

		/// @brief Returns the absolute value of the vector.
		/// @return Absolute vector.
		constexpr Vector4 absolute() const {
			return Vector4(
				CTL::Math::abs(x),
				CTL::Math::abs(y),
				CTL::Math::abs(z),
				CTL::Math::abs(w)
			);
		}

		/// @brief Returns the X and Y components.
		/// @return X andY components.
		constexpr Vector2 xy() const {
			return Vector2(x, y);
		}

		/// @brief Returns the X, Y and Z components.
		/// @return X, Y and Z components.
		constexpr Vector3 xyz() const {
			return Vector3(x, y, z);
		}

		/// @brief Returns the vector in (W, Z, Y, X) form.
		/// @return Vector in (W, Z, Y, X) form.
		constexpr Vector4 wzyx() const {
			return Vector4(w, z, y, x);
		}

		/// @brief Returns the vector in (W, X, Y, Z) form.
		/// @return Vector in (W, X, Y, Z) form.
		constexpr Vector4 wxyz() const {
			return Vector4(w, x, y, z);
		}

		/// @brief Returns the vector compensated by the W value.
		/// @return Compensated vector.
		constexpr Vector4 compensated() const {
			return Vector4(xyz() / w, w);
		}
};
#pragma pack(pop)

/// @brief `Vector2` shorthand.
typedef Vector2 Vec2;
/// @brief `Vector3` shorthand.
typedef Vector3 Vec3;
/// @brief `Vector4` shorthand.
typedef Vector4 Vec4;

/// @brief Decays to a vector of the given dimension.
/// @tparam D Dimension. If zero, decays to `void`.
template<usize D>
using Vector	= CTL::Meta::NthType<D, void, float, Vector2, Vector3, Vector4>;

/// @brief Decays to a vector of the given dimension.
/// @tparam D Dimension.
template<usize D>
using Vec		= Vector<D>;

/// @brief Transformation representation.
/// @tparam TPosition Position component.
/// @tparam TRotation Rotation component.
/// @tparam TScale Scaling component.
template <
	Type::Ex::Math::Vector::Vector		TPosition,
	Type::Ex::Math::Vector::Vectorable	TRotation	= TPosition,
	Type::Ex::Math::Vector::Vector		TScale		= TPosition
>
struct Transform {
	/// @brief Position component type.
	typedef TPosition	PositionType;
	/// @brief Rotation component type.
	typedef TRotation	RotationType;
	/// @brief Scaling component type.
	typedef TScale		ScaleType;

	/// @brief Default constructor.
	constexpr Transform():
		position(0),
		rotation(0),
		scale(1) {}

	/// @brief Constructs the transform with a set of transforms.
	/// @param position Position.
	/// @param rotation Rotation.
	/// @param scale Scaling.
	constexpr Transform(PositionType const& position, RotationType const& rotation, ScaleType const& scale):
		position(position),
		rotation(rotation),
		scale(scale) {}

	/// @brief Returns a transform with no position and rotation, and a scaling of 1.
	/// @return Identity transform.
	constexpr static Transform identity() {return Transform(0, 0, 1);}

	/// @brief Returns the inverse of the transform.
	/// @return Inverse of transform.
	constexpr Transform inverted() const {return Transform(-position, -rotation, TScale(1) / scale);}

	/// @brief Inverts the transform.
	/// @return Reference to self.
	constexpr Transform& invert() const {
		position *= -1;
		rotation *= -1;
		scale = TScale(1) / scale;
		return *this;
	}

	/// @brief Position transform.
	TPosition	position	= TPosition(0);
	/// @brief Rotation transform.
	TRotation	rotation	= TRotation(0);
	/// @brief Scaling transform.
	TScale		scale		= TScale(1);
};

/// @brief Two-dimensional transformation.
using Transform2D = Transform<Vector2, float, Vector2>;
/// @brief Three-dimensional transformation.
using Transform3D = Transform<Vector3, Vector3, Vector3>;

/// @brief 3D rotation axis.
enum class RotationAxis: usize {
	POS_X,
	POS_Y,
	POS_Z,
	NEG_X,
	NEG_Y,
	NEG_Z
};

/// @brief Rotates a 2D Vector around the origin by a given angle.
/// @param vec Vector to rotate.
/// @param angle Angle to rotate by.
/// @return Rotated vector.
constexpr Vector2 rotateV2(Vector2 vec, float const angle) {
	// Calculate the sine & cosine of the angle
	float sinA, cosA;
	CTL::Math::sincos(angle, sinA, cosA);
	// Calculate the rotation around the Z axis (i.e. 2D rotation)
	vec.x = vec.x * cosA - vec.y * sinA;
	vec.y = vec.x * sinA + vec.y * cosA;
	// Return rotated vector
	return vec;
}

/// @brief Rotates a given 3D Vector around the origin's axes by given angles.
/// @param vec Vector to rotate.
/// @param angle Angle to rotate by.
/// @return Rotated vector.
constexpr Vector3 rotateV3(Vector3 vec, Vector3 const& angle) {
	/*
	* Based off of 3D Rotation Matrices:
	* https://en.wikipedia.org/wiki/Rotation_matrix#General_rotations
	* Most likely follows Tait-Bryan Angles:
	* https://en.wikipedia.org/wiki/Euler_angles#Tait%E2%80%93Bryan_angles
	*/
	// Get a copy of the vector
	Vector3 res = vec;
	#ifndef CTL_EX_SIMPLIFIED_ROTATION_MATH
	// Get sines and cosines
	float sinX, cosX, sinY, cosY, sinZ, cosZ;
	CTL::Math::sincos(angle.x, sinX, cosX);
	CTL::Math::sincos(angle.y, sinY, cosY);
	CTL::Math::sincos(angle.z, sinZ, cosZ);
	// Calculate Z axis
	res.x = (cosZ * res.x) - (sinZ * res.y);
	res.y = (sinZ * res.x) + (cosZ * res.y);
	// Calculate Y axis
	res.x = (cosY * res.x) + (sinY * sinZ * res.y) + (sinY * cosZ * res.z);
	res.z = (-sinY * cosX * res.x) + (cosX * cosY * res.y) + (sinX * cosY * res.z);
	// Calculate X axis
	res.y = (cosX * res.y) - (sinX * res.z);
	res.z = (sinX * res.y) + (cosX * res.z);
	#else
	// Rotation buffer
	Vector2 buf;
	// Rotate around Z axis
	buf = rotateV2(res.xy(), angle.z);
	res = Vector3(buf.x, buf.y, res.z);
	// Rotate around Y axis
	buf = rotateV2(res.xz(), angle.y);
	res = Vector3(buf.x, res.y, buf.y);
	// Rotate around X axis
	buf = rotateV2(res.yz(), angle.z);
	res = Vector3(res.x, buf.x, buf.y);
	#endif
	// Return result
	return res;
}

/// @brief Rotates a given 3D Vector around two of the origin's axis by two respective angles.
/// @param vec Vector to rotate.
/// @param angle Angles to rotate by.
/// @param exclude Axis to exclude in rotation. By default, it is the X axis.
/// @return Rotated angle.
constexpr Vector3 rotateV3(Vector3 const& vec, Vector2 const& angle, RotationAxis const& exclude = RotationAxis::POS_X) {
	switch (exclude) {
	case RotationAxis::POS_X:
	case RotationAxis::NEG_X:
		return rotateV3(vec, Vector3(0, angle.x, angle.y));
	case RotationAxis::POS_Y:
	case RotationAxis::NEG_Y:
	default:
		return rotateV3(vec, Vector3(angle.x, 0, angle.y));
	case RotationAxis::POS_Z:
	case RotationAxis::NEG_Z:
		return rotateV3(vec, Vector3(angle.x, angle.y, 0));
	}
}

/// @brief Gets a 2D normal at a given angle relative to the origin.
/// @param angle Angle to get normal for.
/// @return Normal to angle.
constexpr Vector2 angleV2(float const angle) {
	float s, c;
	CTL::Math::sincos(angle, s, c);
	return Vector2(c, -s);
}

/// @brief Gets a 3D normal at a given angle around one of the origin's axis.
/// @param angle Angle to get normal for.
/// @param axis Axis to use as rotation pivot.
/// @return Normal to angle.
constexpr Vector3 angleV3(float const angle, RotationAxis const& axis = RotationAxis::NEG_Z) {
	float s, c;
	CTL::Math::sincos(angle, s, c);
	switch (axis) {
	case RotationAxis::POS_X:
	case RotationAxis::NEG_X:
		return Vector3(0, c, -s);
	case RotationAxis::POS_Y:
	case RotationAxis::NEG_Y:
		return Vector3(c, 0, -s);
	default:
	case RotationAxis::POS_Z:
	case RotationAxis::NEG_Z:
		return Vector3(c, -s, 0);
	}
}

/// @brief Gets a 3D nomal, pointing towards a given axis, rotated at a given angle.
/// @param angle Angle to get normal for.
/// @param axis Axis for starting normal to point towards.
/// @return Normal to angle.
constexpr Vector3 angleV3(Vector3 const& angle, RotationAxis const& axis = RotationAxis::NEG_Z) {
	switch (axis) {
	case RotationAxis::POS_X:
		return rotateV3(Vector3(+1,0,0), angle);
	case RotationAxis::NEG_X:
		return rotateV3(Vector3(-1,0,0), angle);
	case RotationAxis::POS_Y:
		return rotateV3(Vector3(0,+1,0), angle);
	case RotationAxis::NEG_Y:
		return rotateV3(Vector3(0,-1,0), angle);
	case RotationAxis::POS_Z:
		return rotateV3(Vector3(0,0,+1), angle);
	default:
	case RotationAxis::NEG_Z:
		return rotateV3(Vector3(0,0,-1), angle);
	}
}

// Scale-rotation-position transformation

/// @brief Transforms a given vector by a given position, rotation and scale.
/// @param vec Vector to transform.
/// @param pos Position to transform by.
/// @param rot Rotation to transform by.
/// @param scale Scale to transform by.
/// @return Transformed vector.
constexpr Vector3 srpTransform(Vector3 vec, Vector3 const& pos, Vector3 const& rot, Vector3 const& scale = Vector3(1.0)) {
	vec *= scale;
	vec = rotateV3(vec, rot);
	vec += pos;
	return vec;
}

/// @brief Transforms a given vector by a given position, rotation and scale.
/// @param vec Vector to transform.
/// @param pos Position to transform by.
/// @param rot Rotation to transform by.
/// @param scale Scale to transform by.
/// @return Transformed vector.
constexpr Vector2 srpTransform(Vector2 vec, Vector2 const& pos, float const rot, Vector2 const& scale = Vector2(1.0)) {
	vec *= scale;
	vec = rotateV2(vec, rot);
	vec += pos;
	return vec;
}

/// @brief Transforms a given set of vectors by a given position, rotation and scale.
/// @param vec Vectors to transform.
/// @param pos Position to transform by.
/// @param rot Rotation to transform by.
/// @param scale Scale to transform by.
/// @return Transformed vectors.
constexpr List<Vector3> srpTransform(List<Vector3> vec, Vector3 const& pos, Vector3 const& rot, Vector3 const& scale = Vector3(1.0)) {
	for (usize i = 0; i < vec.size(); i++) {
		vec[i] = srpTransform(vec[i], pos, rot, scale);
	}
	return vec;
}

/// @brief Transforms a given set of vectors by a given position, rotation and scale.
/// @param vec Vectors to transform.
/// @param pos Position to transform by.
/// @param rot Rotation to transform by.
/// @param scale Scale to transform by.
/// @return Transformed vectors.
constexpr List<Vector2> srpTransform(List<Vector2> vec, Vector2 const& pos, float const rot, Vector2 const& scale = Vector2(1.0)) {
	for (usize i = 0; i < vec.size(); i++) {
		vec[i] = srpTransform(vec[i], pos, rot, scale);
	}
	return vec;
}

/// @brief Reflects a given normal in accordance to a surface normal.
/// @param normal Normal to reflect.
/// @param surface Surface to reflect by.
/// @return Reflected vector.
constexpr Vector2 reflect(Vector2 const& normal, Vector2 const& surface) {
	return surface * (normal - 2.0 * normal.dot(surface));
}

/// @brief Linearly interpolates two vectors by a certain amount.
/// @tparam T Vector type.
/// @tparam T2 Interpolation factor type.
/// @param from Beginning vector.
/// @param to End vector.
/// @param by Interpolation factor.
/// @return Interpolation between vectors.
template<
	Type::Ex::Math::Vector::Vector T,
	Type::Ex::Math::Vector::Vectorable T2 = T
>
constexpr T angleLerp(T const& from, T const& to, T2 const& by)
requires (Type::Equal<T, T2> || Type::Number<T2>) {
	T dist = (to - from) % TAU;
	dist = ((dist * 2.0) % TAU) - dist;
	return from + dist * by;
}

/// @brief Gets the "center" of a given set of points.
/// @tparam T Vector type.
/// @param points Points to get the center of.
/// @return Center of points.
template<Type::Ex::Math::Vector::Vector T>
constexpr T center(List<T> const& points) {
	T result;
	for (T const& p: points)
		result += p;
	result /= points.size();
	return result;
}

typedef List<Vector2> Points2D;
typedef List<Vector3> Points3D;
typedef List<Vector4> Points4D;

/// @brief Transforms a given vector by a given set of transforms.
/// @param vec Vector to transform.
/// @param trans Transformation to apply.
/// @return Transformed vector.
constexpr Vector2 srpTransform(Vector2 const& vec, Transform2D const& trans) {
	return srpTransform(
		vec,
		trans.position,
		trans.rotation,
		trans.scale
	);
}

/// @brief Transforms a given vector by a given set of transforms.
/// @param vec Vector to transform.
/// @param trans Transformation to apply.
/// @return Transformed vector.
constexpr Vector3 srpTransform(Vector3 const& vec, Transform3D const& trans) {
	return srpTransform(
		vec,
		trans.position,
		trans.rotation,
		trans.scale
	);
}

static_assert(sizeof(Vector2) == (sizeof(float) * 2), "Vector2 has some size issues...");
static_assert(sizeof(Vector3) == (sizeof(float) * 3), "Vector3 has some size issues...");
static_assert(sizeof(Vector4) == (sizeof(float) * 4), "Vector4 has some size issues...");

#pragma GCC diagnostic pop

}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_LINALG_VECTOR_H
