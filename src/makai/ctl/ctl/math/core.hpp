#ifndef CTL_MATH_CORE_H
#define CTL_MATH_CORE_H

#include <cmath>
#include "../typetraits/traits.hpp"
#include "../namespace.hpp"
#include "../typeinfo.hpp"
#include "../ctypes.hpp"
#include "../container/string.hpp"
#include "../container/pair.hpp"
#include "../container/function.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Math-specific type traits.
namespace Type::Math {
	/// @brief Type must be arithmetically operatable.
	template <typename T>
	concept Operatable = Mutable<T> && Arithmetic<T, T>;
}

/// @brief Math functions.
namespace Math {
	/// @brief Math constants.
	namespace Constants {
		/// @brief Approximate value of half of the square root of 2.
		constexpr const double	HSQRT2		= SQRT2 / 2.0;
		/// @brief Approximate value of a quarter of the square root of 2.
		constexpr const double	QSQRT2		= SQRT2 / 4.0;
		/// @brief Approximate value of half of the square root of 3.
		constexpr const double	HSQRT3		= SQRT2 / 2.0;
		/// @brief Approximate value of a quarter of the square root of 3.
		constexpr const double	QSQRT3		= SQRT2 / 4.0;
		/// @brief Approximate value of half of pi.
		constexpr const double	HPI			= PI / 2.0;
		/// @brief Approximate value of a quarter of pi.
		constexpr const double	QPI			= PI / 4.0;
		/// @brief Approximate value of 180 divided by pi.
		constexpr const double	DEGRAD		= 180.0 / PI;
		/// @brief Approximate value of this random number I made up.
		constexpr const double	MARIBEL		= EULER - (11.0 - 6.0 * SQRT2) / 5.0;
		/// @brief Approximate value of tau divided by phi.
		constexpr const double	TAUPHI		= TAU / PHI;
	}

	/// @brief Returns the sign of the value.
	/// @tparam T Value type.
	/// @param val Value to check.
	/// @return Sign of the value.
	template<Type::Math::Operatable T = float>
	constexpr T sign(T const& val)
	requires (
		Type::Comparable::All<T, T>
	&&	Type::Constructible<T, float>
	) {
		return (val < 0 ? -1 : (val > 0 ? +1 : 0));
	}

	/// @brief Returns the smallest of the two values.
	/// @tparam T Value type.
	/// @param a First value.
	/// @param b Second value.
	/// @return Smallest of the two values.
	template<Type::Math::Operatable T = float>
	constexpr T min(T const& a, T const& b)
	requires Type::Comparable::Lesser<T, T> {
		return ((a < b) ? a : b);
	}

	/// @brief Returns the largest of the two values.
	/// @tparam T Value type.
	/// @param a First value.
	/// @param b Second value.
	/// @return Largest of the two values.
	template<Type::Math::Operatable T = float>
	constexpr T max(T const& a, T const& b)
	requires Type::Comparable::Greater<T, T> {
		return ((a > b) ? a : b);
	}

	/// @brief Clamps the value between a given range.
	/// @tparam T Value type.
	/// @param a Value to clamp.
	/// @param min Minimum.
	/// @param max Maximum.
	/// @return Clamped value.
	template<Type::Math::Operatable T = float>
	constexpr T clamp(T const& a, T const& min, T const& max)
	requires Type::Comparable::All<T, T> {
		return ((a < min) ? min : ((a > max) ? max : a));
	}

	/// @brief Converts the value, in degrees, to radians.
	/// @tparam T Value type.
	/// @param deg Value to convert.
	/// @return Value in radians.
	template<Type::Math::Operatable T = float>
	constexpr T radians(T const& deg) requires Type::Arithmetic<T, float> {
		return (T)(deg / DEGRAD);
	}

	/// @brief Converts the value, in radians, to degrees.
	/// @tparam T Value type.
	/// @param rad Value to convert.
	/// @return Value in degrees.
	template<Type::Math::Operatable T = float>
	constexpr T degrees(T const& rad) requires Type::Arithmetic<T, float> {
		return (T)(rad * DEGRAD);
	}

	/// @brief Linearly interpolates between two values.
	/// @tparam T Value type.
	/// @param from Start value.
	/// @param to End value.
	/// @param by How much to interpolate by.
	/// @return Result of the interpolation.
	template<Type::Math::Operatable T = float>
	constexpr T lerp(T const& from, T const& to, T const& by) {
		return (T)(from + by * (to - from));
	}

	/// Taken from Godot Engine, I think.
	/// @brief Linearly interpolates between two angles.
	/// @tparam T Number type.
	/// @param from Start angle.
	/// @param to End angle.
	/// @param by How much to interpolate by.
	/// @return Result of the interpolation.
	template<Type::Number T = float>
	constexpr T angleLerp(T const& from, T const& to, T const& by) {
		T dist	= fmod(to - from, TAU);
		dist	= fmod(2.0 * dist, TAU) - dist;
		return (T)(from + dist * by);
	}

	/// @brief Steps a value, such that `step` is always a multiple of it.
	/// @tparam T Number type.
	/// @param val Value to step.
	/// @param step Step size.
	/// @return Stepped value.
	template<Type::Number T = float>
	constexpr T step(T const& val, T const& step) {
		if (step != 0)
			return (T)(floor(val / step + 0.5) * step);
		return val;
	}

	/// @brief Rounds a number down.
	/// @tparam T Number type.
	/// @param val Value to round.
	/// @return Rounded value.
	template<Type::Number T = float>
	constexpr T floor(T const& val) {
		return (T)(::floor(val));
	}

	/// @brief Rounds a number down.
	/// @tparam T Number type.
	/// @param val Value to round.
	/// @param decimals Decimal spaces to round down to.
	/// @return Rounded value.
	template<Type::Number T = float>
	constexpr T floor(T const& val, ssize const decimals) {
		// Get rounding factor
		T zeros = pow<floatmax>(10, decimals);
		// Floor it
		return (T)(floor(val * zeros) / zeros);
	}

	/// @brief Rounds a number up.
	/// @tparam T Number type.
	/// @param val Value to round.
	/// @return Rounded value.
	template<Type::Number T = float>
	constexpr T ceil(T const& val) {
		return (T)(::ceil(val));
	}

	/// @brief Rounds a number up.
	/// @tparam T Number type.
	/// @param val Value to round.
	/// @param decimals Decimal spaces to round up to.
	/// @return Rounded value.
	template<Type::Number T = float>
	constexpr T ceil(T const& val, int const decimals) {
		// Get rounding factor
		T zeros = pow<floatmax>(10, decimals);
		// Ceil it
		return (T)(::ceil(val * zeros) / zeros);
	}

	/// @brief Rounds a number to the nearest integer.
	/// @tparam T Number type.
	/// @param val Value to round.
	/// @return Rounded value.
	template<Type::Number T = float>
	constexpr T round(T const& val) {
		// Add 1/2 & floor it
		return (T)(::floor(val + 0.5));
	}

	/// @brief Rounds a number up to the nearest value.
	/// @tparam T Number type.
	/// @param val Value to round.
	/// @param decimals Decimal spaces to round to.
	/// @return Rounded value.
	template<Type::Number T = float>
	constexpr T round(T const& val, int const decimals) {
		// Get rounding factor
		T zeros = pow<floatmax>(10, decimals);
		// Add 1/2 & floor it
		return (T)(::floor((val + 0.5) * zeros) / zeros);
	}
	
	template<Type::Number T = float>
	constexpr T fmult(T const& val, T const& mult) {
		return ::floor(val / (double)mult) * mult;
	}

	template<Type::Number T = float>
	constexpr T cmult(T const& val, T const& mult) {
		return ::ceil(val / (double)mult) * mult;
	}

	/// @brief Returns the remainder of a division between two numbers.
	/// @tparam T Number type.
	/// @param a Left-hand side.
	/// @param b Right-hand side.
	/// @return Modulo between `a` and `b`.
	template<Type::Number T = float>
	constexpr T mod(T const& a, T const& b) {
		return fmod(a, b);
	}

	/// @brief Wraps a value, such that it is between 0 and 1.
	/// @tparam T Number type.
	/// @param val Value to wrap around.
	/// @return Wrapped value.
	template<Type::Number T = float>
	constexpr T ramp(T const& val) {
		return val - round(val);
	}

	/// @brief Returns the fractional part of a number.
	/// @tparam T Number type.
	/// @param val Value to get the fractional part from.
	/// @return Fractional part of the number.
	template<Type::Number T = float>
	constexpr T fract(T const& val) {
		return val - ::floor(val);
	}

	/// @brief Wraps an integer, such that it stays between bounds.
	/// @tparam T Integer type.
	/// @param val Value to wrap.
	/// @param min Minimum.
	/// @param max Maximum.
	/// @return Wrapped value.
	template<Type::Integer T = int>
	constexpr T wrapi(T const& val, T const& min, T const& max) {
		return ((val - min) % (max - min)) + min;
	}

	/// @brief Wraps a number, such that it stays between bounds.
	/// @tparam T Number type.
	/// @param val Value to wrap.
	/// @param min Minimum.
	/// @param max Maximum.
	/// @return Wrapped value.
	template<Type::Number T = float>
	constexpr T wrap(T const& val, T const& min, T const& max) {
		return mod(val - min, max - min) + min;
	}

	/// @brief Returns the Nth root of a number.
	/// @tparam T Number type.
	/// @param val Value to get the Nth root from.
	/// @param root Root to take from the value.
	/// @return The Nth root of the value.
	template<Type::Number T = float>
	constexpr T nroot(T const& val, T const& root) {
		return exp(log(root) / val);
	}

	/// @brief Returns the Nth root of N.
	/// @tparam T Number type.
	/// @param val Value to take its own root from.
	/// @return The Nth root of N.
	template<Type::Number T = float>
	constexpr T nrtn(T const& val) {
		return nroot(val, val);
	}

	/// @brief Returns the absolute sine of a value.
	/// @tparam T Number type.
	/// @param val Value to get the absolute sine of.
	/// @return Absolute sine of the value.
	template<Type::Number T = float> constexpr T absin(T const& val) {return abs(sin(val));}
	/// @brief Returns the absolute cosine of a value.
	/// @tparam T Number type.
	/// @param val Value to get the absolute cosine of.
	/// @return Absolute cosine of the value.
	template<Type::Number T = float> constexpr T abcos(T const& val) {return abs(cos(val));}
	/// @brief Returns the absolute tangent of a value.
	/// @tparam T Number type.
	/// @param val Value to get the absolute tangent of.
	/// @return Absolute tangent of the value.
	template<Type::Number T = float> constexpr T abtan(T const& val) {return abs(tan(val));}
	/// @brief Returns the absolute sine and cosine of a value.
	/// @tparam T Number type.
	/// @param val Value to get the absolute sine and cosine of.
	/// @param sin resulting absolute sine.
	/// @param sin resulting absolute cosine.
	template<Type::Number T = float> constexpr void absincos(T const& val, T& sin, T& cos) {
		sincos(val, sin, cos);
		sin = abs(sin);
		cos = abs(cos);
	}

	/// @brief Reflects a given angle in relation to a surface along another angle.
	/// @param a Angle to reflect.
	/// @param s Surface angle.
	/// @return Reflected angle.
	constexpr float reflect(float const a, float const s) {
		return (2.0 * s) - a;
	}

	/// @brief Generates an interpolation function for between two values.
	/// @tparam T Value type.
	/// @param from Start value.
	/// @param to End value.
	/// @return Interpolation function for between two values.
	template<Type::Math::Operatable T>
	constexpr inline Function<T(T const&)> getInterpolationFunction(T const& from, T const& to) {
		return [=](T const& by) -> T {return (T)lerp(from, to, by);};
	}
	
	/// @brief Multiplies an unsigned integer to a power of two.
	/// @tparam T Integer type.
	/// @param val Value to multiply.
	/// @param power Power of two to multiply by.
	/// @return Multiplied value.
	template<Type::Integer T = int>
	constexpr inline usize uipow2(T const& val, usize const power) {
		return val << power;
	}

	/// @brief Returns the Nth bit of an integer.
	/// @tparam T Integer type.
	/// @param val Value to get the bit from.
	/// @param bit Bit to get.
	/// @return Whether the bit is set.
	template<Type::Integer T = int>
	constexpr inline bool bit(T const& val, usize const bit) {
		return (val & (0x1 << bit));
	}

	/// @brief Returns a fraction, and a whole power of two, for a given number.
	/// @tparam T Number type.
	/// @param val Number to extract from.
	/// @return Pair containing results.
	template <Type::Number T>
	constexpr Pair<int, T> frexp(T const& val) {
		int pow2 = 0;
		T rem = std::frexp((float)val, &pow2);
		return Pair<intmax, T>{pow2, rem};
	}

	/// @brief Multiplies a number by a power of two.
	/// @tparam T Number type.
	/// @param val Value to multiply.
	/// @param exp Power of two to multiply by.
	/// @return Multiplied value.
	template <Type::Number T>
	constexpr T ldexp(T const& val, intmax const& exp) {
		return (T)::ldexp((double)val, exp);
	}

	/// @brief Returns the factorial for a given integer.
	/// @tparam T Integer type.
	/// @param val Value to get the factorial for.
	/// @return Factorial of value.
	template <Type::Integer T = int>
	constexpr T factorial(T val) {
		if (val < 2) return val;
		T res = val;
		while (--val) res *= val;
		return res;
	}
}

/// @brief Math literals.
namespace Literals::Math {

// "Degrees to Radians" literal conversion

/// @brief Degree literal.
/// @return Value in radians.
constexpr float			operator "" degf(long double d)			{return CTL::Math::radians<float>(d);		}
/// @brief Degree literal.
/// @return Value in radians.
constexpr float			operator "" degf(unsigned long long d)	{return CTL::Math::radians<float>(d);		}
/// @brief Degree literal.
/// @return Value in radians.
constexpr double		operator "" degd(long double d)			{return CTL::Math::radians<double>(d);		}
/// @brief Degree literal.
/// @return Value in radians.
constexpr double		operator "" degd(unsigned long long d)	{return CTL::Math::radians<double>(d);		}
/// @brief Degree literal.
/// @return Value in radians.
constexpr long double	operator "" degld(long double d)		{return CTL::Math::radians<long double>(d);	}
/// @brief Degree literal.
/// @return Value in radians.
constexpr long double	operator "" degld(unsigned long long d)	{return CTL::Math::radians<long double>(d);	}
/// @brief Degree literal.
/// @return Value in radians.
constexpr float			operator "" deg(long double d)			{return CTL::Math::radians<float>(d);		}
/// @brief Degree literal.
/// @return Value in radians.
constexpr float			operator "" deg(unsigned long long d)	{return CTL::Math::radians<float>(d);		}

// "Radians to Degrees" literal conversion

#ifdef CTL_INCLUDE_RADIAN_CONVERSION_LITERALS
/// @brief Radian literal.
/// @return Value in degrees.
constexpr float			operator "" radf(long double r)			{return CTL::Math::degrees<float>(r);		}
/// @brief Radian literal.
/// @return Value in degrees.
constexpr float			operator "" radf(unsigned long long r)	{return CTL::Math::degrees<float>(r);		}
/// @brief Radian literal.
/// @return Value in degrees.
constexpr double		operator "" radd(long double r)			{return CTL::Math::degrees<double>(r);		}
/// @brief Radian literal.
/// @return Value in degrees.
constexpr double		operator "" radd(unsigned long long r)	{return CTL::Math::degrees<double>(r);		}
/// @brief Radian literal.
/// @return Value in degrees.
constexpr long double	operator "" radld(long double r)		{return CTL::Math::degrees<long double>(r);	}
/// @brief Radian literal.
/// @return Value in degrees.
constexpr long double	operator "" radld(unsigned long long r)	{return CTL::Math::degrees<long double>(r);	}
/// @brief Radian literal.
/// @return Value in degrees.
constexpr float			operator "" rad(long double r)			{return CTL::Math::degrees<float>(r);		}
/// @brief Radian literal.
/// @return Value in degrees.
constexpr float			operator "" rad(unsigned long long r)	{return CTL::Math::degrees<float>(r);		}
#endif // CTL_INCLUDE_RADIAN_CONVERSION_LITERALS

}

CTL_NAMESPACE_END

#pragma GCC diagnostic pop

#endif // CTL_MATH_CORE_H
