#ifndef CTL_MATH_BIGNUMBER_H
#define CTL_MATH_BIGNUMBER_H

#include "core.hpp"

CTL_NAMESPACE_BEGIN

namespace Math {
	template <Type::Number T, usize N>
	struct BigNumber {
		using DataType				= T;
		constexpr static usize SIZE	= N;
		using InternalType			= DataType[N];
	};
}

CTL_NAMESPACE_END

#endif