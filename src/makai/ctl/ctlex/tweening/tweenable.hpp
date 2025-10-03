#ifndef CTL_EX_TWEENING_TWEENABLE_H
#define CTL_EX_TWEENING_TWEENABLE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/math/math.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Tween-specific type constraints.
namespace Type::Ex::Tween {
	/// @brief Type must be operatable, and must be constructible from a single float.
	template <typename T>
	concept Tweenable = CTL::Type::Math::Operatable<T> && CTL::Type::Constructible<T, float>;
}

CTL_EX_NAMESPACE_END

#endif