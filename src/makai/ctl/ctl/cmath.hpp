#ifndef CTL_CMATH_H
#define CTL_CMATH_H

#include "namespace.hpp"
#include "typetraits/traits.hpp"
#include "typetraits/cast.hpp"
#include "algorithm/bitwise.hpp"

CTL_NAMESPACE_BEGIN

namespace Math {

template<usize B>
struct IEEFloat;

/// @brief 32-bit IEE float representation.
template<>
struct IEEFloat<32> {
	uint32 sign:	1;
	int32 mantissa:	8;
	int32 exponent:	23;
};

/// @brief 64-bit IEE float representation.
template<>
struct IEEFloat<64> {
	uint64 sign:	1;
	int64 mantissa:	11;
	int64 exponent:	53;
};

/// @brief Mathematical constants.
namespace Constants {
	/// @brief Approximate value of the square root of 2, to the first 64 decimal places.
	constexpr const ldouble	SQRT2	= 1.4142135623730950488016887242096980785696718753769480731766797380;
	/// @brief Approximate value of the square root of 3, to the first 64 decimal places.
	constexpr const ldouble	SQRT3	= 1.7320508075688772935274463415058723669428052538103806280558069795;
	/// @brief Approximate value of the natural logarithm of 2, to the first 64 decimal places.
	constexpr const ldouble	LN2		= 0.69314718055994530941723212145817656807550013436025525412068000949;
	/// @brief Approximate value of the natural logarithm of 10, to the first 64 decimal places.
	constexpr const ldouble LN10	= 2.3025850929940456840179914546843642076011014886287729760333279010;
	/// @brief Approximate value of pi, to the first 64 decimal places.
	constexpr const ldouble	PI		= 3.1415926535897932384626433832795028841971693993751058209749445923;
	/// @brief Approximate value of tau (double of pi), to the first 64 decimal places.
	constexpr const ldouble	TAU		= 6.2831853071795864769252867665590057683943387987502116419498891846;
	/// @brief Approximate value of euler's constant, to the first 64 decimal places.
	constexpr const ldouble	EULER	= 2.7182818284590452353602874713526624977572470936999595749669676277;
	/// @brief Approximate value of the golden ratio, to the first 64 decimal places.
	constexpr const ldouble	PHI		= 1.6180339887498948482045868343656381177203091798057628621354486227;
}

/// @brief Returns if `a` is between `b - epsilon` and `b + epsilon`.
/// @tparam F Floating point type.
/// @param a Value to compare.
/// @param b Value to compare to.
/// @param epsilon Precision.
/// @return Whether they are close enough.
template<Type::Real F = double>
constexpr bool compare(F const a, F const b, F const epsilon = 1.0e-5) {
	return (a >= b - epsilon) && (a <= b + epsilon);
}

/// @brief Returns the absolute of a given value.
/// @tparam T Number type.
/// @param v Value to get the absolute of.
/// @return Absolute value.
template<Type::Number T = double>
constexpr T abs(T const v) {
	return (v < 0) ? -v : v;
}

/// @brief Returns euler's number raised to the given value.
/// @tparam F Floating point type.
/// @param value Value to use as exponent.
/// @param precision Precision of approximation. By default, it is `sizeof(F) * 4`.
/// @note `precision` is only used if `CTL_MATH_DO_NOT_USE_BUILTINS` is defined.
/// @return euler's number raised to the given value.
template<Type::Real F = double>
constexpr F exp(F const value, usize precision = sizeof(F) * 4) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if constexpr (Type::Equal<F, float>)
		return __builtin_expf(value);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_exp(value);
	else
		return __builtin_expl(value);
	#else
	// Based off of https://codingforspeed.com/using-faster-exponential-approximation/
	F out = 1.0 + value / static_cast<F>(static_cast<usize>(1) << precision);
	for (usize i = 0; i < precision; ++i)
		out *= out;
	return out;
	#endif
}

static_assert(compare<double>(exp<double>(1.0), Constants::EULER));

/// @brief Returns the natural logarithm of a given number.
/// @tparam F Floating point type.
/// @param value Value to get the natural logarithm for.
/// @return Natural logarithm of number.
template<Type::Real F = double>
constexpr F log(F value) {
	if (!value) return 0;
	if constexpr (Type::Equal<F, float>)
		return __builtin_logf(value);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_log(value);
	else
		return __builtin_logl(value);
}

static_assert(compare<double>(log<double>(4.0), 1.38629436112));

/// @brief Returns the logarithm of a given base for a given number.
/// @tparam F Floating point type.
/// @param value Value to get the logarithm for.
/// @param base Base to get logarithm in.
/// @return Logarithm of number.
template<Type::Real F = double>
constexpr F logn(F const value, F const base) {
	return log<F>(value) / log<F>(base);
}

/// @brief Returns the base-2 logarithm for a given number.
/// @tparam F Floating point type.
/// @param value Value to get the logarithm for.
/// @return Logarithm of number.
template<Type::Real F = double>
constexpr F log2(F const value) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if (!value) return 0;
	if constexpr (Type::Equal<F, float>)
		return __builtin_log2f(value);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_log2(value);
	else
		return __builtin_log2l(value);
	#else
	return log<F>(value) / static_cast<F>(Constants::LN2);
	#endif
}

/// @brief Returns the base-10 logarithm for a given number.
/// @tparam F Floating point type.
/// @param value Value to get the logarithm for.
/// @return Logarithm of number.
template<Type::Real F = double>
constexpr F log10(F const value) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if (!value) return 0;
	if constexpr (Type::Equal<F, float>)
		return __builtin_log10f(value);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_log10(value);
	else
		return __builtin_log10l(value);
	#else
	return log<F>(value) / static_cast<F>(Constants::LN10);
	#endif
}

/// @brief Calculates a value raised to a given power.
/// @tparam F Floating point type.
/// @param value Value to raise.
/// @param power Power to raise by.
/// @return Value raised to the given power.
template<Type::Real F = double>
constexpr F pow(F const value, F power, usize const precision = sizeof(F) * 4) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if (!value) return 0;
	if constexpr (Type::Equal<F, float>)
		return __builtin_powf(value, power);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_pow(value, power);
	else
		return __builtin_powl(value, power);
	#else
	return exp<F>(power*log<F>(value), precision);
	#endif
}

static_assert(compare<double>(pow<double>(10, 0), 1));
static_assert(compare<double>(pow<double>(10, 1), 10));
static_assert(compare<double>(pow<double>(10, 2), 100));

/// @brief Calculates the root of a number.
/// @tparam F Floating point type.
/// @param value Value to get root for.
/// @param root Root to get.
/// @param precision Precision of approximation. By default, it is `sizeof(F) * 4`.
/// @note `precision` is only used if `CTL_MATH_DO_NOT_USE_BUILTINS` is defined.
/// @return Root of number.
template<Type::Real F = double>
constexpr F root(F const value, F const root, usize const precision = sizeof(F) * 4) {
	return exp<F>(log<F>(value) / root, precision);
}

/// @brief Returns the square root of a number.
/// @tparam F Floating point type.
/// @param value Value to get square root for.
/// @param precision Precision of approximation. By default, it is `sizeof(F) * 4`.
/// @note `precision` is only used if `CTL_MATH_DO_NOT_USE_BUILTINS` is defined.
/// @return Square root of number.
template<Type::Real F = double>
constexpr F sqrt(F const value, usize const precision = sizeof(F) * 4) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if constexpr (Type::Equal<F, float>)
		return __builtin_sqrtf(value);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_sqrt(value);
	else
		return __builtin_sqrtl(value);
	#else
	return root<F>(value, 2, precision);
	#endif
}

static_assert(compare<double>(sqrt<double>(4.0), 2));

/// @brief Returns the remainder of a number divided by another.
/// @tparam F Floating point type.
/// @param val Value to divide.
/// @param mod Value to divide by.
/// @return Remainder.
template<Type::Real F = double>
constexpr F fmod(F const val, F const mod) {
	if constexpr (Type::Equal<F, float>)
		return __builtin_fmodf(val, mod);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_fmod(val, mod);
	else
		return __builtin_fmodl(val, mod);
}

/// @brief Returns the given angle, wrapped between `-PI` and `+PI`.
/// @tparam F Floating point type.
/// @param angle Angle to wrap.
/// @return Wrapped angle.
template<Type::Real F = double>
constexpr F rmod(F const& angle) {
	return fmod<F>(angle + static_cast<F>(Constants::PI), Constants::TAU) - static_cast<F>(Constants::PI);
}

/// @brief Calculates the arc tangent of a number.
/// @tparam F Floating point type.
/// @param x Number to get the arc tangent for.
/// @return Arc tangent of number.
/// @note Based off of https://stackoverflow.com/a/42542593
template<Type::Real F = double>
constexpr F atan(F const value) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if constexpr (Type::Equal<F, float>)
		return __builtin_atanf(value);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_atan(value);
	else
		return __builtin_atanl(value);
	#else
	return 8.430893743524 * value / (3.2105332277903100 + sqrt<F>(27.2515970979709 + 29.3591908371266 * value * value));
	#endif
}

/// @brief Calculates the arc tangent of Y/X.
/// @tparam F Floating point type.
/// @param x X.
/// @param y Y.
/// @return Arc tangent of Y/X.
template<Type::Real F = double>
constexpr F atan2(F const y, F const x) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if constexpr (Type::Equal<F, float>)
		return __builtin_atan2f(y, x);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_atan2(y, x);
	else
		return __builtin_atan2l(y, x);
	#else
	if (!x && !y)	return 0;
	if (x == 0)		return (static_cast<F>(Constants::PI)/2) * (y < 0 ? -1 : +1);
	if (x < 0)		return atan<F>(y/x) + static_cast<F>(Constants::PI) * (y < 0 ? -1 : +1);
	return atan<F>(y/x);
	#endif
}

static_assert(compare<double>(atan2<double>(0, 1), 0));
static_assert(compare<double>(atan2<double>(1, 0), Constants::PI/2));
static_assert(compare<double>(atan2<double>(1, 1), Constants::PI/4));

/// @brief Calculates both sine and cosine of a given angle.
/// @tparam F Floating point type.
/// @param angle Angle to get sine and cosine for.
/// @param sin Output of sine of angle.
/// @param cos Output of cosine of angle.
template<Type::Real F = double>
constexpr void sincos(F const angle, F& sin, F& cos) {
	if constexpr (Type::Equal<F, float>)
		return __builtin_sincosf(angle, &sin, &cos);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_sincos(angle, &sin, &cos);
	else
		return __builtin_sincosl(angle, &sin, &cos);
}

/// @brief Calculates the sine of a given angle.
/// @tparam F Floating point type.
/// @param angle Angle to get sine for.
/// @return Sine of angle.
template<Type::Real F = double>
constexpr F sin(F const angle) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if constexpr (Type::Equal<F, float>)
		return __builtin_sinf(angle);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_sin(angle);
	else
		return __builtin_sinl(angle);
	#else
	F out, _;
	sincos<F>(angle, out, _);
	return out;
	#endif
}

/// @brief Calculates the cosine of a given angle.
/// @tparam F Floating point type.
/// @param angle Angle to get cosine for.
/// @return Cosine of angle.
template<Type::Real F = double>
constexpr F cos(F const angle) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if constexpr (Type::Equal<F, float>)
		return __builtin_cosf(angle);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_cos(angle);
	else
		return __builtin_cosl(angle);
	#else
	F out, _;
	sincos<F>(angle, _, out);
	return out;
	#endif
}

/// @brief Calculates the tangent of a given angle.
/// @tparam F Floating point type.
/// @param angle Angle to get tangent for.
/// @return Tangent of angle.
template<Type::Real F = double>
constexpr F tan(F const angle) {
	#ifndef CTL_MATH_DO_NOT_USE_BUILTINS
	if constexpr (Type::Equal<F, float>)
		return __builtin_tanf(angle);
	else if constexpr (Type::Equal<F, double>)
		return __builtin_tan(angle);
	else
		return __builtin_tanl(angle);
	#else
	F s, c;
	sincos<F>(angle, s, c);
	return s / c;
	#endif
}

static_assert(compare<double>(sin<double>(0), 0));
static_assert(compare<double>(cos<double>(0), 1));
static_assert(compare<double>(sin<double>(Constants::PI/2), 1));
static_assert(compare<double>(cos<double>(Constants::PI/2), 0));
static_assert(compare<double>(sin<double>(Constants::PI), 0));
static_assert(compare<double>(cos<double>(Constants::PI), -1));

}

/// @brief Numeric literals.
namespace Literals::Numbers {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wliteral-suffix"
	constexpr ldouble operator "" sqrt2(ullong v)	{return static_cast<ldouble>(v) * Math::Constants::SQRT2;	}
	constexpr ldouble operator "" sqrt2(ldouble v)	{return v * Math::Constants::SQRT2;							}
	constexpr ldouble operator "" sqrt3(ullong v)	{return static_cast<ldouble>(v) * Math::Constants::SQRT3;	}
	constexpr ldouble operator "" sqrt3(ldouble v)	{return v * Math::Constants::SQRT3;							}
	constexpr ldouble operator "" ln2(ullong v)		{return static_cast<ldouble>(v) * Math::Constants::LN2;		}
	constexpr ldouble operator "" ln2(ldouble v)	{return v * Math::Constants::LN10;							}
	constexpr ldouble operator "" ln10(ullong v)	{return static_cast<ldouble>(v) * Math::Constants::LN2;		}
	constexpr ldouble operator "" ln10(ldouble v)	{return v * Math::Constants::LN10;							}
	constexpr ldouble operator "" pi(ullong v)		{return static_cast<ldouble>(v) * Math::Constants::PI;		}
	constexpr ldouble operator "" pi(ldouble v)		{return v * Math::Constants::PI;							}
	constexpr ldouble operator "" tau(ullong v)		{return static_cast<ldouble>(v) * 2pi;						}
	constexpr ldouble operator "" tau(ldouble v)	{return v * 2pi;											}
	constexpr ldouble operator "" euler(ullong v)	{return static_cast<ldouble>(v) * Math::Constants::EULER;	}
	constexpr ldouble operator "" euler(ldouble v)	{return v * Math::Constants::EULER;							}
	constexpr ldouble operator "" phi(ullong v)		{return static_cast<ldouble>(v) * Math::Constants::PHI;		}
	constexpr ldouble operator "" phi(ldouble v)	{return v *Math:: Constants::PHI;							}
	#pragma GCC diagnostic pop
}

CTL_NAMESPACE_END

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
/// @brief Custom floating point literal.
constexpr float			operator "" f(unsigned long long v)		{return v;	}
/// @brief Custom floating point literal.
constexpr float			operator "" F(unsigned long long v)		{return v;	}
/// @brief Custom double precision point literal.
constexpr double		operator "" d(unsigned long long v)		{return v;	}
/// @brief Custom double precision point literal.
constexpr double		operator "" D(unsigned long long v)		{return v;	}
/// @brief Custom quadruple precision point literal.
constexpr long double	operator "" ld(long double v)			{return v;	}
/// @brief Custom quadruple precision point literal.
constexpr long double	operator "" LD(long double v)			{return v;	}
/// @brief Custom quadruple precision point literal.
constexpr long double	operator "" ld(unsigned long long v)	{return v;	}
/// @brief Custom quadruple precision point literal.
constexpr long double	operator "" LD(unsigned long long v)	{return v;	}

using namespace CTL::Math::Constants;

#endif // CTL_CMATH_H
