#ifndef CTL_INTERFACE_CORE_H
#define CTL_INTERFACE_CORE_H

#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Interface for a compile-time-able stored value.
/// @tparam T Value type.
template<class T>
struct IConstValue {
	/// @brief Returns the stored value.
	/// @return Stored value.
	constexpr virtual T value() const = 0;
};

/// @brief Interface for a stored value.
/// @tparam T Value type.
template<class T>
struct IValue {
	/// @brief Returns the stored value.
	/// @return Stored value.
	virtual T value() const = 0;
};

CTL_NAMESPACE_END

#endif // CTL_INTERFACE_CORE_H