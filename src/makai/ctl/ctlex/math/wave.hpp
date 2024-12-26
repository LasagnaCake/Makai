#ifndef CTL_EX_MATH_WAVE_H
#define CTL_EX_MATH_WAVE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/container/string.hpp"
#include "../../ctl/container/function.hpp"
#include "../../ctl/math/core.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace Math::Wave {
	namespace {
		using
			::CTL::Math::absin,
			::CTL::Math::abcos,
			::CTL::Math::ramp,
			::CTL::Math::round,
			::CTL::Type::Math::Operatable,
			::CTL::Math::fract
		;
	}

	/// @brief Wave function wrapper type.
	/// @tparam T Number type.
	template<Type::Number T = float>
	using WaveFunction = Function<T(T const, usize const)>;

	#define WAVE_FUNCTION(NAME) template<Type::Number T = float> constexpr T NAME(T const t, usize const lod = 8)
 
	WAVE_FUNCTION(square)	{return sin(t) < 0 ? T(-1) : T(+1);	}

	WAVE_FUNCTION(sine)			{return sin(t);							}
	WAVE_FUNCTION(halfSine)		{return sin(ramp(t/PI  + 2.5) * PI);	}
	WAVE_FUNCTION(absSine)		{return absin(t);						}
	WAVE_FUNCTION(absHalfSine)	{return abs(halfSine(t));				}

	WAVE_FUNCTION(triangle) {
		float as, ac;
		CTL::Math::absincos(t*0.5 + (HPI*0.5), as, ac);
		return (as-ac);
	}
	WAVE_FUNCTION(halfTriangle)		{return ramp(t/PI  + 2.5) * 2.0;								}
	WAVE_FUNCTION(absTriangle)		{return abs(triangle(t));										}
	WAVE_FUNCTION(absHalfTriangle)	{return abs(halfTriangle(t));									}

	#undef WAVE_FUNCTION

	namespace Binary {
		#define BINARY_WAVE_FUNCTION(NAME)\
			template<Type::Number T>\
			constexpr T NAME(T const t, usize const lod) {\
				return (T)round<T>(::CTL::Ex::Math::Wave::NAME(t) * lod) / lod;\
			}

		BINARY_WAVE_FUNCTION(sine			)
		BINARY_WAVE_FUNCTION(halfSine		)
		BINARY_WAVE_FUNCTION(absSine		)
		BINARY_WAVE_FUNCTION(absHalfSine	)

		BINARY_WAVE_FUNCTION(triangle			)
		BINARY_WAVE_FUNCTION(halfTriangle		)
		BINARY_WAVE_FUNCTION(absTriangle		)
		BINARY_WAVE_FUNCTION(absHalfTriangle	)

		#undef BINARY_WAVE_FUNCTION
	}
}

namespace Math::Noise {
	namespace {
		using
			::CTL::Math::fract,
			::CTL::Type::Math::Operatable
		;
	}

	template<Type::Number T = float>
	using NoiseFunction = Function<T(T const, usize const)>;

	#define NOISE_FUNCTION(NAME) template<Type::Number T = float> constexpr T NAME(T const t, usize const seed = 8)

	NOISE_FUNCTION(simple) {
		return fract<T>(sin(t * ::CTL::Math::mod((seed+sizeof(T)) / 12.9898, 78.233) * 43758.5453));
	}

	NOISE_FUNCTION(gold) {
		return fract<T>(tan((t*PHI - t) * seed));
	}

	NOISE_FUNCTION(super) {
		return gold<T>(simple<T>(t), seed);
	}

	#undef NOISE_FUNCTION
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_MATH_WAVE_H
