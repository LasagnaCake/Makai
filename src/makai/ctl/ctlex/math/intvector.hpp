#ifndef CTL_EX_MATH_INTVECTOR_H
#define CTL_EX_MATH_INTVECTOR_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/math/core.hpp"
#include "../../ctl/order.hpp"
#include "vector.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

CTL_EX_NAMESPACE_BEGIN

namespace Math {
	struct IntVector2;
	struct IntVector3;
	struct IntVector4;
}

/// @brief IntVector-specific type constraints.
namespace Type::Ex::Math::IntVector {
	/// @brief Type must be a valid 2D vector.
	template<typename T>
	concept IntVector2 = requires {
		requires	CTL::Type::Equal<T, ::CTL::Ex::Math::IntVector2>;
	};

	/// @brief Type must be a valid 3D vector.
	template<typename T>
	concept IntVector3 = requires {
		requires	CTL::Type::Equal<T, ::CTL::Ex::Math::IntVector3>;
	};

	/// @brief Type must be a valid 4D vector.
	template<typename T>
	concept IntVector4 = requires {
		requires	CTL::Type::Equal<T, ::CTL::Ex::Math::IntVector4>;
	};

	/// @brief Type must be a valid vector.
	template<typename T>
	concept IntVector =
		IntVector2<T>
	||	IntVector3<T>
	||	IntVector4<T>
	;

	/// @brief Type must be a vector of some kind.
	template<typename T>
	concept IntVectorable =
		CTL::Type::Number<T>
	||	CTL::Type::OneOf<
			T,
			::CTL::Ex::Math::IntVector2,
			::CTL::Ex::Math::IntVector3,
			::CTL::Ex::Math::IntVector4
	>;

	/// @brief Types must form a valid vector operation.
	template<class A, class B>
	concept Operatable =
		IntVectorable<A>
	&&	IntVectorable<B>
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
		Type::Ex::Math::IntVector::IntVectorable A,
		Type::Ex::Math::IntVector::Operatable<A> B
	>
	using IntVectorType = CTL::Meta::DualType<Type::Number<B>, A, B>;

	/// @brief Decays to the vector type with the most components.
	/// @tparam A Possible type.
	/// @tparam B Possible type.
	template<
		Type::Ex::Math::IntVector::IntVectorable A,
		Type::Ex::Math::IntVector::IntVectorable B
	>
	using LargestIntVectorType = CTL::Meta::DualType<
		sizeof(A) < sizeof(B),
		IntVectorType<A, B>,
		IntVectorType<B, A>
	>;
}

/// @brief Universal vector unary plus operator.
/// @tparam A IntVector type.
/// @param v IntVector to operate on.
/// @return Result of the operation.
template<Type::Ex::Math::IntVector::IntVector T>
constexpr T operator+(T const& v) {
	if constexpr (Type::Equal<T, IntVector2>)
		return T(v.x, v.y);
	else if constexpr (Type::Equal<T, IntVector3>)
		return T(v.x, v.y, v.z);
	else if constexpr (Type::Equal<T, IntVector4>)
		return T(v.x, v.y, v.z, v.w);
}

/// @brief Universal vector unary minus operator.
/// @tparam A IntVector type.
/// @param v IntVector to operate on.
/// @return Result of the operation.
template<Type::Ex::Math::IntVector::IntVector T>
constexpr T operator-(T const& v) {
	if constexpr (Type::Equal<T, IntVector2>)
		return T(-v.x, -v.y);
	else if constexpr (Type::Equal<T, IntVector3>)
		return T(-v.x, -v.y, -v.z);
	else if constexpr (Type::Equal<T, IntVector4>)
		return T(-v.x, -v.y, -v.z, -v.w);
}


/// @brief Universal vector addition operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVectorable A,
	Type::Ex::Math::IntVector::Operatable<A> B
>
constexpr Meta::IntVectorType<A, B> operator+(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestIntVectorType<A, B>(a) + Meta::LargestIntVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, IntVector2>)
		return A(a.x + b.x, a.y + b.y);
	else if constexpr (Type::Equal<A, IntVector3>)
		return A(a.x + b.x, a.y + b.y, a.z + b.z);
	else if constexpr (Type::Equal<A, IntVector4>)
		return A(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

/// @brief Universal vector subtraction operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVectorable A,
	Type::Ex::Math::IntVector::Operatable<A> B
>
constexpr Meta::IntVectorType<A, B> operator-(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestIntVectorType<A, B>(a) - Meta::LargestIntVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, IntVector2>)
		return A(a.x - b.x, a.y - b.y);
	else if constexpr (Type::Equal<A, IntVector3>)
		return A(a.x - b.x, a.y - b.y, a.z - b.z);
	else if constexpr (Type::Equal<A, IntVector4>)
		return A(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

/// @brief Universal vector multiplication operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVectorable A,
	Type::Ex::Math::IntVector::Operatable<A> B
>
constexpr Meta::IntVectorType<A, B> operator*(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestIntVectorType<A, B>(a) * Meta::LargestIntVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, IntVector2>)
		return A(a.x * b.x, a.y * b.y);
	else if constexpr (Type::Equal<A, IntVector3>)
		return A(a.x * b.x, a.y * b.y, a.z * b.z);
	else if constexpr (Type::Equal<A, IntVector4>)
		return A(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

/// @brief Universal vector division operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVectorable A,
	Type::Ex::Math::IntVector::Operatable<A> B
>
constexpr Meta::IntVectorType<A, B> operator/(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestIntVectorType<A, B>(a) / Meta::LargestIntVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, IntVector2>)
		return A(a.x / b.x, a.y / b.y);
	else if constexpr (Type::Equal<A, IntVector3>)
		return A(a.x / b.x, a.y / b.y, a.z / b.z);
	else if constexpr (Type::Equal<A, IntVector4>)
		return A(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

/// @brief Universal vector modulo operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVectorable A,
	Type::Ex::Math::IntVector::Operatable<A> B
>
constexpr Meta::IntVectorType<A, B> operator%(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestIntVectorType<A, B>(a) % Meta::LargestIntVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, IntVector2>)
		return A(fmod(a.x, b.x), fmod(a.y, b.y));
	else if constexpr (Type::Equal<A, IntVector3>)
		return A(fmod(a.x, b.x), fmod(a.y, b.y), fmod(a.z, b.z));
	else if constexpr (Type::Equal<A, IntVector4>)
		return A(fmod(a.x, b.x), fmod(a.y, b.y), fmod(a.z, b.z), fmod(a.w, b.w));
}

/// @brief Universal vector exponentiation operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVectorable A,
	Type::Ex::Math::IntVector::Operatable<A> B
>
constexpr Meta::IntVectorType<A, B> operator^(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestIntVectorType<A, B>(a) ^ Meta::LargestIntVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, IntVector2>)
		return A(pow(a.x, b.x), pow(a.y, b.y));
	else if constexpr (Type::Equal<A, IntVector3>)
		return A(pow(a.x, b.x), pow(a.y, b.y), pow(a.z, b.z));
	else if constexpr (Type::Equal<A, IntVector4>)
		return A(pow(a.x, b.x), pow(a.y, b.y), pow(a.z, b.z), pow(a.w, b.w));
}

/// @brief Universal vector addition assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVector A,
	Type::Ex::Math::IntVector::IntVectorable B
>
constexpr Meta::IntVectorType<A, B> operator+=(A& a, B const& b) {
	return a = a + b;
}

/// @brief Universal vector subtraction assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVector A,
	Type::Ex::Math::IntVector::IntVectorable B
>
constexpr Meta::IntVectorType<A, B> operator-=(A& a, B const& b) {
	return a = a - b;
}

/// @brief Universal vector multiplication assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVector A,
	Type::Ex::Math::IntVector::IntVectorable B
>
constexpr Meta::IntVectorType<A, B> operator*=(A& a, B const& b) {
	return a = a * b;
}

/// @brief Universal vector division assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVector A,
	Type::Ex::Math::IntVector::IntVectorable B
>
constexpr Meta::IntVectorType<A, B> operator/=(A& a, B const& b) {
	return a = a / b;
}

/// @brief Universal vector modulo assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVector A,
	Type::Ex::Math::IntVector::IntVectorable B
>
constexpr Meta::IntVectorType<A, B> operator%=(A& a, B const& b) {
	return a = a % b;
}

/// @brief Universal vector exponentiation assignment operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVector A,
	Type::Ex::Math::IntVector::IntVectorable B
>
constexpr Meta::IntVectorType<A, B> operator^=(A& a, B const& b) {
	return a = a ^ b;
}

/// @brief Universal vector comparison operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVectorable A,
	Type::Ex::Math::IntVector::Operatable<A> B
>
constexpr bool operator==(A const& a, B const& b) {
	if constexpr (Type::Different<A, B>)
		return Meta::LargestIntVectorType<A, B>(a) == Meta::LargestIntVectorType<A, B>(b);
	else if constexpr (Type::Equal<A, IntVector2>)
		return a.x == b.x && a.y == b.y;
	else if constexpr (Type::Equal<A, IntVector3>)
		return a.x == b.x && a.y == b.y && a.z == b.z;
	else if constexpr (Type::Equal<A, IntVector4>)
		return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

/// @brief Universal vector threeway comparison operator.
/// @tparam A Left side type.
/// @tparam B Right side type.
/// @param a Left side of the operation.
/// @param b Right side of the operation.
/// @return Result of the operation.
template<
	Type::Ex::Math::IntVector::IntVectorable A,
	Type::Ex::Math::IntVector::Operatable<A> B
>
constexpr Meta::IntVectorType<A, B>::OrderType operator<=>(A const& a, B const& b) {
	using Order = Meta::IntVectorType<A, B>::Order;
	if constexpr (Type::Different<A, B>)
		return Meta::LargestIntVectorType<A, B>(a) <=> Meta::LargestIntVectorType<A, B>(b);
	typename Meta::IntVectorType<A, B>::OrderType	ord = a.x <=> b.x;
	if constexpr (Type::Equal<A, IntVector2>)
		if (ord == Order::EQUAL)				ord = a.y <=> b.y;
	if constexpr (Type::Equal<A, IntVector3>)
		if (ord == Order::EQUAL)				ord = a.z <=> b.z;
	if constexpr (Type::Equal<A, IntVector4>)
		if (ord == Order::EQUAL)				ord = a.w <=> b.w;
	return ord;
}

#pragma pack(push, 1)
/// @brief Two-dimensional vector.
struct IntVector2: Ordered {
public:
	using Ordered::OrderType, Ordered::Order;

	/// @brief IntVector components.
	union {
		struct {ssize x, y;		};
		struct {ssize nx, ny;	};
		struct {ssize u, v;		};
		ssize data[2] = {0};
	};

	/// @brief Default constructor.
	constexpr IntVector2() {}

	/// @brief Constructs the vector's components with a starting value.
	/// @param v Value to set.
	constexpr IntVector2(ssize const v):
		x(v), y(v) {}

	/// @brief Constructs the vector's components with a set of values.
	/// @param x X component.
	/// @param y Y component.
	constexpr IntVector2(ssize const x, ssize const y):
		x(x), y(y) {}

	/// @brief Copy constructor.
	/// @tparam T IntVector type.
	/// @param vec IntVector to copy from.
	template<Type::OneOf<IntVector2, IntVector3, IntVector4> T>
	constexpr IntVector2(T const& vec): IntVector2(vec.x, vec.y) {}

	/// @brief Constructs the vector from an array of values.
	/// @param data Values to construct from.
	constexpr IntVector2(As<ssize[2]> const& data):
		IntVector2(data[0], data[1]) {}

	/// @brief Destructor.
	constexpr ~IntVector2() {}

	/// @brief Returns a "all-zeroes" vector.
	/// @return "All-zeroes" vector.
	constexpr static IntVector2 ZERO()		{return 0;					}
	/// @brief Returns an "all-ones" vector.
	/// @return "All-ones" vector.
	constexpr static IntVector2 ONE()		{return 1;					}
	/// @brief Returns a vector pointing towards the positive X axis.
	/// @return IntVector pointing towards +X.
	constexpr static IntVector2 RIGHT()	{return IntVector2(+1, +0);	}
	/// @brief Returns a vector pointing towards the negative X axis.
	/// @return IntVector pointing towards -X.
	constexpr static IntVector2 LEFT()		{return IntVector2(-1, +0);	}
	/// @brief Returns a vector pointing towards the positive Y axis.
	/// @return IntVector pointing towards +Y.
	constexpr static IntVector2 UP()		{return IntVector2(+0, +1);	}
	/// @brief Returns a vector pointing towards the negative Y axis.
	/// @return IntVector pointing towards -Y.
	constexpr static IntVector2 DOWN()		{return IntVector2(+0, -1);	}

	/// @brief Gets a component at a given index.
	/// @param pos Index of the component.
	/// @return Reference to component.
	constexpr ssize& operator[](usize const pos)		{if (pos > 1) return data[0]; return data[pos];}
	/// @brief Gets a component at a given index.
	/// @param pos Index of the component.
	/// @return Value of the component.
	constexpr ssize operator[](usize const pos) const	{if (pos > 1) return data[0]; return data[pos];}

	/// @brief Gets the sum of the vector's components.
	/// @returns Sum of components.
	constexpr ssize sum() const {
		return x + y;
	}

	/// @brief Gets the average of the vector's components.
	/// @returns Average of components.
	constexpr ssize average() const {
		return sum() / 2;
	}

	/// @brief Gets the smallest of the vector's components.
	/// @returns Smallest of component.
	constexpr ssize min() const {
		return (x < y) ? x : y;
	}

	/// @brief Gets the biggest of the vector's components.
	/// @returns Biggest component.
	constexpr ssize max() const {
		return (x > y) ? x : y;
	}

	/// @brief Returns a vector containing the smallest components between it and another vector.
	/// @param other Other vector.
	/// @return Smallest vector.
	constexpr IntVector2 min(IntVector2 const& other) const {
		return IntVector2(
			CTL::Math::min(x, other.x),
			CTL::Math::min(y, other.y)
		);
	}

	/// @brief Returns a vector containing the biggest components between it and another vector.
	/// @param other Other vector.
	/// @return Biggest vector.
	constexpr IntVector2 max(IntVector2 const& other) const {
		return IntVector2(
			CTL::Math::max(x, other.x),
			CTL::Math::max(y, other.y)
		);
	}

	/// @brief Gets the vector's length.
	/// @brief Length of vector.
	constexpr float length() const {
		return CTL::Math::sqrt<float>((x * x) + (y * y));
	}

	/// @brief Gets the vector's squared length.
	/// @brief Squared length of vector.
	constexpr ssize lengthSquared() const {
		return ((x * x) + (y * y));
	}

	/// @brief Gets the normalized vector.
	/// @returns Normalized vector.	
	constexpr IntVector2 normalized() const {
		if (*this != 0)
			return *this / length();
		else
			return *this;
	}

	/// @brief Normalizes the vector.
	/// @returns Reference to self.
	constexpr IntVector2& normalize() {
		return *this = normalized();
	}

	/// @brief Gets the distance to another vector.
	/// @param vec IntVector to get distance to.
	/// @return Distance between vectors.
	constexpr ssize distanceTo(IntVector2 const& vec) const {
		IntVector2 diff = vec - *this;
		return diff.length();
	}

	/// @brief Gets the squared distance to another vector.
	/// @param vec IntVector to get distance to.
	/// @return Distance between vectors.
	constexpr ssize squaredDistanceTo(IntVector2 const& vec) const {
		IntVector2 diff = vec - *this;
		return diff.lengthSquared();
	}

	/// @brief Gets the vector's angle.
	/// @return Angle of vector.
	constexpr float angle() const {
		return CTL::Math::atan2<float>(x, y);
	}

	/// @brief Gets the vector's angle to another vector.
	/// @param vec IntVector to get angle to.
	/// @return Angle to vector.
	constexpr ssize angleTo(IntVector2 const& vec) const {
		IntVector2 diff = vec - *this;
		return diff.angle();
	}

	/// @brief Gets a normal pointing towards another vector.
	/// @param vec IntVector to get normal to.
	/// @return Normal to vector.
	constexpr IntVector2 normalTo(IntVector2 const& vec) const {
		IntVector2 diff = vec - *this;
		return diff.normalized();
	}


	/// @brief Clamps the vector between two values.
	/// @param min Minimum.
	/// @param max Maximum.
	/// @return Reference to self.
	constexpr IntVector2& clamp(IntVector2 const& min, IntVector2 const& max) {
		x = ::CTL::Math::clamp(x, min.x, max.x);
		y = ::CTL::Math::clamp(y, min.y, max.y);
		return *this;
	}

	/// @brief Returns the vector clamped between two values.
	/// @param min Minimum.
	/// @param max Maximum.
	/// @return Clamped vector.
	constexpr IntVector2 clamped(IntVector2 const& min, IntVector2 const& max) const {
		return IntVector2(
			::CTL::Math::clamp(x, min.x, max.x),
			::CTL::Math::clamp(y, min.y, max.y)
		);
	}

	/// @brief Gets the tangent of the vector.
	/// @return Tangent of vector.
	constexpr ssize tangent() const {
		return x / y;
	}

	/// @brief Returns the dot product with another vector.
	/// @param vec IntVector to get dot product with.
	/// @return Dot product between vectors.
	constexpr ssize dot(IntVector2 const& vec) const {
		IntVector2 mult = (*this) * vec;
		return mult.x + mult.y;
	}

	/// @brief Returns the cross product with another vector.
	/// @param vec IntVector to get cross product with.
	/// @return Cross product between vectors.
	constexpr ssize cross(IntVector2 const& vec) const {
		return (x * vec.y) - (y * vec.x);
	}

	/// @brief Returns the "cross product" with another vector via the triple product.
	/// @param vec IntVector to get cross product with.
	/// @return "Cross product" between vectors.
	constexpr IntVector2 fcross(IntVector2 const& vec) const {
		return tri(vec, vec);
	}

	/// @brief Returns the triple cross product `A x (B x C)` with two other vectors.
	/// @param b IntVector to get triple cross product with.
	/// @param c IntVector to get triple cross product with.
	/// @return Triple cross product.
	constexpr IntVector2 tri(IntVector2 const& b, IntVector2 const& c) const {
		return (b * dot(c)) - (c * dot(b));
	}

	/// @brief Returns the inverse triple cross product `(A x B) x C` with two other vectors.
	/// @param b IntVector to get triple cross product with.
	/// @param c IntVector to get triple cross product with.
	/// @return Triple cross product.
	constexpr IntVector2 itri(IntVector2 const& b, IntVector2 const& c) const {
		return - c.tri(*this, b);
	}

	/// @brief Returns this vector projected in another vector.
	/// @param vec IntVector to project in.
	/// @return Result of the operation.
	constexpr IntVector2 projected(IntVector2 const& vec) const {
		return (vec.dot(*this) / vec.dot(vec)) * vec;
	}

	/// @brief Projects this vector in another vector.
	/// @param vec IntVector to project in.
	/// @return Reference to self.
	constexpr IntVector2& project(IntVector2 const& vec) {
		return *this = projected(vec);
	}

	/// @brief Returns the absolute value of the vector.
	/// @return Absolute vector.
	constexpr IntVector2 absolute() const {
		return IntVector2(
			abs(x),
			abs(y)
		);
	}

	/// @brief Returns the vector in (Y, X) form.
	/// @return IntVector in (Y, X) form.
	constexpr IntVector2 yx() const {
		return IntVector2(y, x);
	}

	/// @brief Returns the integer vector as a `Vector2`.
	/// @return Vector as `Vector2`.
	constexpr Vector2 toVector2() const	{return {x, y};			}

	/// @brief Returns the integer vector as a `Vector2`.
	/// @return Vector as `Vector2
	constexpr operator Vector2() const	{return toVector2();	}
};

/// @brief Three-dimensional vector.
struct IntVector3: Ordered {
public:
	using Ordered::OrderType, Ordered::Order;
	
	/// @brief IntVector components.
	union {
		struct {ssize x, y, z;		};
		struct {ssize nx, ny, nz;	};
		struct {ssize r, g, b;		};
		struct {ssize u, v, t;		};
		ssize data[3] = {0};
	};

	/// @brief Default constructor.
	constexpr IntVector3() {}

	/// @brief Constructs the vector's components with a starting value.
	/// @param v Value to set.
	constexpr IntVector3(ssize v):
		x(v), y(v), z(v) {}

	/// @brief Constructs the vector's components from a set of values.
	/// @param x X component.
	/// @param y Y component.
	/// @param z Z component. By default, it is 0.
	constexpr IntVector3(ssize const x, ssize const y, ssize const z = 0.0):
		x(x), y(y), z(z) {}

	/// @brief Copy constructor.
	/// @tparam T IntVector type.
	/// @param vec IntVector to copy from.
	template<Type::OneOf<IntVector3, IntVector4> T>
	constexpr IntVector3(T const& vec):
		IntVector3(vec.x, vec.y, vec.z) {}

	/// @brief Constructs the vector's components from a vector and a value.
	/// @param vec IntVector to use for X and Y components.
	/// @param z Z component. By default, it is 0.
	constexpr IntVector3(IntVector2 const& vec, ssize z = 0):
		IntVector3(vec.x, vec.y, z) {}

	/// @brief Constructs the vector from an array of values.
	/// @param data Values to construct from.
	constexpr IntVector3(As<ssize[3]> const& data):
		IntVector3(data[0], data[1], data[2]) {}

	/// @brief Destructor.
	constexpr ~IntVector3() {}

	/// @brief Returns an "all-zeroes" vector.
	/// @return "All-zeroes" vector.
	constexpr static IntVector3 ZERO()		{return 0;						}
	/// @brief Returns an "all-ones" vector.
	/// @return "All-ones" vector.
	constexpr static IntVector3 ONE()		{return 1;						}
	/// @brief Returns a vector pointing towards the positive X axis.
	/// @return IntVector pointing towards +x.
	constexpr static IntVector3 RIGHT()	{return IntVector2::RIGHT();		}
	/// @brief Returns a vector pointing towards the negative X axis.
	/// @return IntVector pointing towards -x.
	constexpr static IntVector3 LEFT()		{return IntVector2::LEFT();		}
	/// @brief Returns a vector pointing towards the positive Y axis.
	/// @return IntVector pointing towards +Y.
	constexpr static IntVector3 UP()		{return IntVector2::UP();			}
	/// @brief Returns a vector pointing towards the negative Y axis.
	/// @return IntVector pointing towards -Y.
	constexpr static IntVector3 DOWN()		{return IntVector2::DOWN();		}
	/// @brief Returns a vector pointing towards the positive Z axis.
	/// @return IntVector pointing towards +Z.
	constexpr static IntVector3 BACK()		{return IntVector3(+0, +0, +1);	}
	/// @brief Returns a vector pointing towards the negative Z axis.
	/// @return IntVector pointing towards -Z.
	constexpr static IntVector3 FRONT()	{return IntVector3(+0, +0, -1);	}

	/// @brief Gets a component at a given index.
	/// @param pos Index of the component.
	/// @return Reference to component.
	constexpr ssize& operator[](usize const pos)		{if (pos > 2) return data[0]; return data[pos];}
	/// @brief Gets a component at a given index.
	/// @param pos Index of the component.
	/// @return Value of component.
	constexpr ssize operator[](usize const pos) const	{if (pos > 2) return data[0]; return data[pos];}

	/// @brief Gets the sum of the vector's components.
	/// @return Sum of components.
	constexpr ssize sum() const {
		return x + y + z;
	}

	/// @brief Gets the average of the vector's components.
	/// @return Average of components.
	constexpr ssize average() const {
		return sum() / 3;
	}

	/// @brief Gets the smallest of the vector's components.
	/// @return Smallest component.
	constexpr ssize min() const {
		return ::CTL::Math::min(::CTL::Math::min(x, y), z);
	}

	/// @brief Gets the biggest of the vector's components.
	/// @return Biggest component.
	constexpr ssize max() const {
		return ::CTL::Math::max(::CTL::Math::max(x, y), z);
	}

	/// @brief Returns a vector containing the smallest components between it and another vector.
	/// @param other Other vector.
	/// @return Smallest vector.
	constexpr IntVector3 min(IntVector3 const& other) const {
		return IntVector3(
			CTL::Math::min(x, other.x),
			CTL::Math::min(y, other.y),
			CTL::Math::min(z, other.z)
		);
	}

	/// @brief Returns a vector containing the biggest components between it and another vector.
	/// @param other Other vector.
	/// @return Biggest vector.
	constexpr IntVector3 max(IntVector3 const& other) const {
		return IntVector3(
			CTL::Math::max(x, other.x),
			CTL::Math::max(y, other.y),
			CTL::Math::min(z, other.z)
		);
	}

	/// @brief Gets the vector's length.
	/// @return Length of vector.
	constexpr float length() const {
		return CTL::Math::sqrt<float>((x * x) + (y * y) + (z * z));
	}

	/// @brief Gets the vector's squared length.
	/// @return Squared ength of vector.
	constexpr ssize lengthSquared() const {
		return ((x * x) + (y * y) + (z * z));
	}

	/// @brief Gets the vector's angle.
	/// @return Angle of vector.
	constexpr Vector3 angle() const {
		Vector3 res;
		float mag = length();
		res.x = acos(Cast::as<float>(x)/mag);
		res.y = acos(Cast::as<float>(y)/mag);
		res.z = acos(Cast::as<float>(z)/mag);
		return res;
	}

	/// @brief Gets the vector's angle to another vector.
	/// @param vec IntVector to get angle to.
	/// @return Angle to vector.
	constexpr Vector3 angleTo(IntVector3 const& vec) const {
		IntVector3 diff = vec - *this;
		return diff.angle();
	}

	/// @brief Gets the normalized vector.
	/// @return Normalized vector.
	constexpr IntVector3 normalized() const {
		if (*this != 0)
			return *this / length();
		else
			return *this;
	}

	/// @brief Normalizes the vector.
	/// @return Reference to self.
	constexpr IntVector3& normalize() {
		return *this = normalized();
	}

	/// @brief Gets a normal pointing towards another vector.
	/// @param vec IntVector to get normal to.
	/// @return Normal to vector.
	constexpr IntVector3 normalTo(IntVector3 const& vec) const {
		IntVector3 diff = vec - *this;
		return diff.normalized();
	}

	/// @brief Gets the distance to another vector.
	/// @param vec IntVector to get distance to.
	/// @return Distance to vector.
	constexpr ssize distanceTo(IntVector3 const& vec) const {
		IntVector3 diff = vec - *this;
		return diff.length();
	}

	/// @brief Gets the squared distance to another vector.
	/// @param vec IntVector to get squared distance to.
	/// @return Squared distance to vector.
	constexpr ssize squaredDistanceTo(IntVector3 const& vec) const {
		IntVector3 diff = vec - *this;
		return diff.lengthSquared();
	}

	/// @brief Clamps the vector between two values.
	/// @param min Minimum.
	/// @param max Maximum.
	/// @return Reference to self.
	constexpr IntVector3& clamp(IntVector3 const& min, IntVector3 const& max) {
		x = ::CTL::Math::clamp(x, min.x, max.x);
		y = ::CTL::Math::clamp(y, min.y, max.y);
		z = ::CTL::Math::clamp(z, min.z, max.z);
		return *this;
	}

	/// @brief Returns the vector clamped between two values.
	/// @param min Minimum.
	/// @param max Maximum.
	/// @return Clamped vector.
	constexpr IntVector3 clamped(IntVector3 const& min, IntVector3 const& max) const {
		return IntVector3(
			::CTL::Math::clamp(x, min.x, max.x),
			::CTL::Math::clamp(y, min.y, max.y),
			::CTL::Math::clamp(z, min.z, max.z)
		);
	}

	/// @brief Returns the dot product with another vector.
	/// @param vec IntVector to get dot product with.
	/// @return Dot product.
	constexpr ssize dot(IntVector3 const& vec) const {
		IntVector3 mult = (*this) * vec;
		return mult.x + mult.y + mult.z;
	}

	/// @brief Returns the cross product with another vector.
	/// @param vec IntVector to get cross product with.
	/// @return Cross product.
	constexpr IntVector3 cross(IntVector3 const& vec) const {
		return IntVector3(
			(y * vec.z) - (z * vec.y),
			(z * vec.x) - (x * vec.z),
			(x * vec.y) - (y * vec.x)
		);
	}

	/// @brief Returns the "cross product" with another vector via the triple product.
	/// @param vec IntVector to get cross product with.
	/// @return "Cross product" between vectors.
	constexpr IntVector3 fcross(IntVector3 const& vec) const {
		return tri(vec, vec);
	}

	/// @brief Returns the triple cross product `A x (B x C)` with two other vectors.
	/// @param b IntVector to get triple cross product with.
	/// @param c IntVector to get triple cross product with.
	/// @return Triple cross product.
	constexpr IntVector3 tri(IntVector3 const& b, IntVector3 const& c) const {
		return (b * dot(c)) - (c * dot(b));
	}

	/// @brief Returns the inverse triple cross product `(A x B) x C` with two other vectors.
	/// @param b IntVector to get triple cross product with.
	/// @param c IntVector to get triple cross product with.
	/// @return Triple cross product.
	constexpr IntVector3 itri(IntVector3 const& b, IntVector3 const& c) const {
		return - c.tri(*this, b);
	}

	/// @brief Returns the mixed (scalar triple) product with two other vectors (`this` dot (`a` cross `b`)).
	/// @param a IntVector to get mixed product with.
	/// @param b IntVector to get mixed product with.
	/// @return Mixed product.
	constexpr ssize mix(IntVector3 const& a, IntVector3 const& b) const {
		return dot(a.cross(b));
	}

	/// @brief Returns this vector projected in another vector.
	/// @param vec IntVector to project in.
	/// @return Result of the operation.
	constexpr IntVector3 projected(IntVector3 const& vec) const {
		return (vec.dot(*this) / vec.dot(vec)) * vec;
	}

	/// @brief Projects this vector in another vector.
	/// @param vec IntVector to project in.
	/// @return Reference to self.
	constexpr IntVector3& project(IntVector3 const& vec) {
		return *this = projected(vec);
	}

	/// @brief Returns the X and Y components.
	/// @return X and Y components.
	constexpr IntVector2 xy() const {
		return IntVector2(x, y);
	}

	/// @brief Returns the Y and Z components.
	/// @return Y and Z components.
	constexpr IntVector2 yz() const {
		return IntVector2(x, y);
	}

	/// @brief Returns the X and Z components.
	/// @return X and Z components.
	constexpr IntVector2 xz() const {
		return IntVector2(x, y);
	}

	/// @brief Returns the vector in (Z, Y, X) form.
	/// @return IntVector in (Z, Y, X) form.
	constexpr IntVector3 zyx() const {
		return IntVector3(z, y, x);
	}

	/// @brief Returns the vector in (X, Z, Y) form.
	/// @return IntVector in (X, Z, Y) form.
	constexpr IntVector3 xzy() const {
		return IntVector3(x, z, y);
	}

	/// @brief Returns the vector in (Y, Z, X) form.
	/// @return IntVector in (Y, Z, X) form.
	constexpr IntVector3 yzx() const {
		return IntVector3(y, z, x);
	}

	/// @brief Returns the absolute value of the vector.
	/// @return Absolute vector.
	constexpr IntVector3 absolute() const {
		return IntVector3(
			abs(x),
			abs(y),
			abs(z)
		);
	}

	/// @brief Returns the integer vector as a `Vector3`.
	/// @return Vector as `Vector3`.
	constexpr Vector3 toVector3() const	{return {x, y, z};		}

	/// @brief Returns the integer vector as a `Vector3`.
	/// @return Vector as `Vector3`.
	constexpr operator Vector3() const	{return toVector3();	}
};

/// @brief Four-dimensional vector.
struct IntVector4: Ordered {
public:
	using Ordered::OrderType, Ordered::Order;

	/// The vector's position.
	union {
		struct {ssize x, y, z, w;		};
		struct {ssize nx, ny, nz, nw;	};
		struct {ssize r, g, b, a;		};
		struct {ssize u, v, t, s;		};
		ssize data[4] = {0};
	};

	/// @brief Default constructor.
	constexpr IntVector4() {}

	/// @brief Constructs the vector's components with a starting value.
	/// @param v Value to set.
	constexpr IntVector4(ssize v):
		x(v), y(v), z(v), w(v) {}

	/// @brief Constructs the vector's components from a set of values.
	/// @param x X component.
	/// @param y Y component.
	/// @param z Z component.
	/// @param w W component. By default, it is 0.
	constexpr IntVector4(ssize const x, ssize const y, ssize const z, ssize const w = 0.0):
		x(x), y(y), z(z), w(w) {}

	/// @brief Constructs the vector's components from a set of 2D vectors.
	/// @param v1 IntVector to use for X and Y components.
	/// @param v2 IntVector to use for Z and W components.
	constexpr IntVector4(IntVector2 const& v1, IntVector2 const& v2):
		IntVector4(v1.x, v1.y, v2.x, v2.y) {}

	/// @brief Copy constructor.
	/// @param vec `IntVector4` to copy from.
	constexpr IntVector4(IntVector4 const& vec):
		IntVector4(vec.x, vec.y, vec.z, vec.w) {}

	/// @brief Constructs the vector's components from a vector and a value.
	/// @param vec IntVector to use for X, Y and Z components.
	/// @param w W component. By default, it is 0.
	constexpr IntVector4(IntVector3 const& vec, ssize w = 0):
		IntVector4(vec.x, vec.y, vec.z, w) {}

	/// @brief Constructs the vector's components from a vector and a value.
	/// @param vec IntVector to use for X and Y components.
	/// @param z Z component. By default, it is 0.
	/// @param w W component. By default, it is 0.
	constexpr IntVector4(IntVector2 const& vec, ssize z = 0, ssize w = 0):
		IntVector4(vec.x, vec.y, z, w) {}

	/// @brief Constructs the vector from an array of values.
	/// @param data Values to construct from.
	constexpr IntVector4(As<ssize[4]> const& data):
		IntVector4(data[0], data[1], data[2], data[3]) {}

	/// @brief Destructor.
	constexpr ~IntVector4() {}

	/// @brief Returns an "all-zeroes" vector.
	/// @return "All-zeroes" vector.
	constexpr static IntVector4 ZERO()		{return 0;							}
	/// @brief Returns an "all-ones" vector.
	/// @return "All-ones" vector.
	constexpr static IntVector4 ONE()		{return 1;							}
	/// @brief Returns a vector pointing towards the positive X axis.
	/// @return vector pointing towards +X.
	constexpr static IntVector4 RIGHT()	{return IntVector2::RIGHT();			}
	/// @brief Returns a vector pointing towards the negative X axis.
	/// @return vector pointing towards -X.
	constexpr static IntVector4 LEFT()		{return IntVector2::LEFT();			}
	/// @brief Returns a vector pointing towards the positive Y axis.
	/// @return vector pointing towards +Y.
	constexpr static IntVector4 UP()		{return IntVector2::UP();				}
	/// @brief Returns a vector pointing towards the negative Y axis.
	/// @return vector pointing towards -Y.
	constexpr static IntVector4 DOWN()		{return IntVector2::DOWN();			}
	/// @brief Returns a vector pointing towards the positive Z axis.
	/// @return vector pointing towards +Z.
	constexpr static IntVector4 BACK()		{return IntVector3::BACK();			}
	/// @brief Returns a vector pointing towards the negative Z axis.
	/// @return vector pointing towards -Z.
	constexpr static IntVector4 FRONT()	{return IntVector3::FRONT();			}
	/// @brief Returns a vector pointing towards the positive W axis.
	/// @return vector pointing towards +W.
	constexpr static IntVector4 FUTURE()	{return IntVector4(+0, +0, +0, +1);	}
	/// @brief Returns a vector pointing towards the negative W axis.
	/// @return vector pointing towards -W.
	constexpr static IntVector4 PAST()		{return IntVector4(+0, +0, +0, -1);	}
	/// @brief Returns a vector pointing towards the positive W axis.
	/// @return vector pointing towards +W.
	constexpr static IntVector4 ANA()		{return IntVector4(+0, +0, +0, +1);	}
	/// @brief Returns a vector pointing towards the negative W axis.
	/// @return vector pointing towards -W.
	constexpr static IntVector4 KATA()		{return IntVector4(+0, +0, +0, -1);	}

	/// @brief Gets a component at a given index.
	/// @param pos Index of the component.
	/// @return Reference to component.
	constexpr ssize& operator[](usize const pos)		{if (pos > 3) return data[0]; return data[pos];}
	/// @brief Gets a component at a given index.
	/// @param pos Index of the component.
	/// @return Value of component.
	constexpr ssize operator[](usize const pos) const	{if (pos > 3) return data[0]; return data[pos];}

	/// @brief Gets the sum of the vector's components.
	/// @return Sum of components.
	constexpr ssize sum() const {
		return x + y + z + w;
	}

	/// @brief Gets the average of the vector's components.
	/// @return Average of components.
	constexpr ssize average() const {
		return sum() / 4;
	}

	/// @brief Gets the smallest of the vector's components.
	/// @return Smallest component.
	constexpr ssize min() const {
		return ::CTL::Math::min(::CTL::Math::min(::CTL::Math::min(x, y), z), w);
	}

	/// @brief Gets the biggest of the vector's components.
	/// @return Biggest component.
	constexpr ssize max() const {
		return ::CTL::Math::max(::CTL::Math::max(::CTL::Math::max(x, y), z), w);
	}

	/// @brief Returns a vector containing the smallest components between it and another vector.
	/// @param other Other vector.
	/// @return Smallest vector.
	constexpr IntVector4 min(IntVector4 const& other) const {
		return IntVector4(
			CTL::Math::min(x, other.x),
			CTL::Math::min(y, other.y),
			CTL::Math::min(z, other.z),
			CTL::Math::min(w, other.w)
		);
	}

	/// @brief Returns a vector containing the biggest components between it and another vector.
	/// @param other Other vector.
	/// @return Biggest vector.
	constexpr IntVector4 max(IntVector4 const& other) const {
		return IntVector4(
			CTL::Math::max(x, other.x),
			CTL::Math::max(y, other.y),
			CTL::Math::min(z, other.z),
			CTL::Math::min(w, other.w)
		);
	}

	/// @brief Gets the vector's length.
	/// @return Length of vector.
	constexpr ssize length() const {
		return CTL::Math::sqrt<float>((x * x) + (y * y) + (z * z) + (w * w));
	}

	/// @brief Gets the vector's squared length.
	/// @return Squared length of vector.
	constexpr ssize lengthSquared() const {
		return ((x * x) + (y * y) + (z * z) + (w * w));
	}

	/// @brief Gets the normalized vector.
	/// @return Normalized vector.
	constexpr IntVector4 normalized() const {
		if (*this != 0)
			return *this / length();
		else
			return *this;
	}

	/// @brief Normalizes the vector.
	/// @return Reference to self.
	constexpr IntVector4& normalize() {
		return *this = normalized();
	}

	/// @brief Gets the distance to another vector.
	/// @param vec IntVector to get distance to.
	/// @return Distance between vectors.
	constexpr ssize distanceTo(IntVector4 const& vec) const {
		IntVector4 diff = vec - *this;
		return diff.length();
	}

	/// @brief Gets the squared distance to another vector.
	/// @param vec IntVector to get squared distance to.
	/// @return Squared distance between vectors.
	constexpr ssize squaredDistanceTo(IntVector4 const& vec) const {
		IntVector4 diff = vec - *this;
		return diff.lengthSquared();
	}

	/// @brief Gets a normal pointing towards another vector.
	/// @param vec IntVector to get normal to.
	/// @return Normal to vector.
	constexpr IntVector4 normalTo(IntVector4 const& vec) const {
		IntVector4 diff = vec - *this;
		return diff.normalized();
	}

	/// @brief Clamps the vector between two values.
	/// @param min Minimum.
	/// @param max Maximum.
	/// @return Reference to self.
	constexpr IntVector4& clamp(IntVector4 const& min, IntVector4 const& max) {
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
	constexpr IntVector4 clamped(IntVector4 const& min, IntVector4 const& max) const {
		return IntVector4(
			::CTL::Math::clamp(x, min.x, max.x),
			::CTL::Math::clamp(y, min.y, max.y),
			::CTL::Math::clamp(z, min.z, max.z),
			::CTL::Math::clamp(w, min.w, max.w)
		);
	}

	/// @brief Returns the dot product with another vector.
	/// @param vec IntVector to get dot product with.
	/// @return Dot product.
	constexpr ssize dot(IntVector4 const& vec) const {
		IntVector4 mult = (*this) * vec;
		return mult.x + mult.y + mult.z + mult.w;
	}

	/// @brief Returns the "cross product" with another vector via the triple product.
	/// @param vec IntVector to get cross product with.
	/// @return "Cross product" between vectors.
	constexpr IntVector4 fcross(IntVector4 const& vec) const {
		return tri(vec, vec);
	}

	/// @brief Returns the triple cross product `A x (B x C)` with two other vectors.
	/// @param b IntVector to get triple cross product with.
	/// @param c IntVector to get triple cross product with.
	/// @return Triple cross product.
	constexpr IntVector4 tri(IntVector4 const& b, IntVector4 const& c) const {
		return (b * dot(c)) - (c * dot(b));
	}

	/// @brief Returns the inverse triple cross product `(A x B) x C` with two other vectors.
	/// @param b IntVector to get triple cross product with.
	/// @param c IntVector to get triple cross product with.
	/// @return Triple cross product.
	constexpr IntVector4 itri(IntVector4 const& b, IntVector4 const& c) const {
		return - c.tri(*this, b);
	}

	/// @brief Returns this vector projected in another vector.
	/// @param vec IntVector to project in.
	/// @return Result of the operation.
	constexpr IntVector4 projected(IntVector4 const& vec) const {
		return (vec.dot(*this) / vec.dot(vec)) * vec;
	}

	/// @brief Projects this vector in another vector.
	/// @param vec IntVector to project in.
	/// @return Reference to self.
	constexpr IntVector4& project(IntVector4 const& vec) {
		return *this = projected(vec);
	}

	/// @brief Returns the absolute value of the vector.
	/// @return Absolute vector.
	constexpr IntVector4 absolute() const {
		return IntVector4(
			abs(x),
			abs(y),
			abs(z),
			abs(w)
		);
	}

	/// @brief Returns the X, Y and Z components.
	/// @return X, Y and Z components.
	constexpr IntVector3 xyz() const {
		return IntVector3(x, y, z);
	}

	/// @brief Returns the vector in (W, Z, Y, X) form.
	/// @return IntVector in (W, Z, Y, X) form.
	constexpr IntVector4 wzyx() const {
		return IntVector4(w, z, y, x);
	}

	/// @brief Returns the vector in (W, X, Y, Z) form.
	/// @return IntVector in (W, X, Y, Z) form.
	constexpr IntVector4 wxyz() const {
		return IntVector4(w, x, y, z);
	}

	/// @brief Returns the vector compensated by the W value.
	/// @return Compensated vector.
	constexpr IntVector4 compensated() const {
		return IntVector4(xyz() / w, w);
	}

	/// @brief Returns the integer vector as a `Vector4`.
	/// @return Vector as `Vector4`.
	constexpr Vector4 toVector4() const	{return {x, y, z, w};	}

	/// @brief Returns the integer vector as a `Vector4`.
	/// @return Vector as `Vector4`.
	constexpr operator Vector4() const	{return toVector4();	}
};
#pragma pack(pop)

/// @brief `IntVector2` shorthand.
typedef IntVector2 IVec2;
/// @brief `IntVector3` shorthand.
typedef IntVector3 IVec3;
/// @brief `IntVector4` shorthand.
typedef IntVector4 IVec4;

/// @brief Decays to a vector of the given dimension.
/// @tparam D Dimension. If zero, decays to `void`.
template<usize D>
using IntVector	= CTL::Meta::NthType<D, void, ssize, IntVector2, IntVector3, IntVector4>;

/// @brief Decays to a vector of the given dimension.
/// @tparam D Dimension.
template<usize D>
using IVec		= IntVector<D>;

static_assert(sizeof(IntVector2) == (sizeof(ssize) * 2), "IntVector2 has some size issues...");
static_assert(sizeof(IntVector3) == (sizeof(ssize) * 3), "IntVector3 has some size issues...");
static_assert(sizeof(IntVector4) == (sizeof(ssize) * 4), "IntVector4 has some size issues...");

#pragma GCC diagnostic pop

}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_MATH_INTVECTOR_H
