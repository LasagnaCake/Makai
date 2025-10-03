#ifndef CTL_EX_TWEENING_PROPERTY_H
#define CTL_EX_TWEENING_PROPERTY_H

#include "../../ctl/exnamespace.hpp"
#include "tweenable.hpp"
#include "interpolate.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Interpolatable property.
/// @tparam T Property type.
template<Type::Ex::Tween::Tweenable T>
struct Property {
	/// @brief Current value.
	T					value		= 0;
	/// @brief Whether to interpolate the property.
	bool				interpolate	= false;
	/// @brief Starting value.
	T					start		= 0;
	/// @brief End value.
	T					stop		= 0;
	/// @brief Interpolation speed.
	float				speed		= 0;
	/// @brief Interpolation function.
	Math::Ease::Mode	ease		= Math::Ease::linear;
	/// @brief Current interpolation factor.
	float				factor		= 0;

	/// @brief Updates the property, and returns its current value.
	/// @return Current value after processing.
	constexpr T next() {
		if (!(interpolate && speed != 0))
			return value;
		factor = CTL::Math::clamp<float>(factor, 0, 1);
		value = ::CTL::Ex::interpolate<T>(start, stop, factor, ease);
		factor += speed;
		return value;
	}

	/// @brief Reverses the property.
	/// @return Reference to self.
	constexpr Property& reverse() {
		CTL::swap(start, stop);
		factor = 1 - factor;
		return *this;
	}
};

CTL_EX_NAMESPACE_END

#endif