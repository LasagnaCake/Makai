#ifndef CTL_TYPETRAITS_NUMBER_H
#define CTL_TYPETRAITS_NUMBER_H

#include "../namespace.hpp"
#include "basictraits.hpp"
#include "decay.hpp"
#include "../algorithm/bitwise.hpp"
#include "../meta/logic.hpp"
#include "../meta/pack.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Number-to-Type decay conversions.
namespace Decay::Number {
	/// @brief Decays an integer value to the nearest signed integer type that can hold it.
	template<ssize S>
	using AsSigned		= Meta::NthType<(byteWidth((S < 0) ? -S : S)-1), int8, int16, int32, int64>;

	/// @brief Decays an integer value to the nearest unsigned integer type that can hold it.
	template<usize S>
	using AsUnsigned	= Meta::NthType<(byteWidth(S)-1), uint8, uint16, uint32, uint64>;
}

CTL_NAMESPACE_END

#endif // CTL_TYPETRAITS_NUMBER_H