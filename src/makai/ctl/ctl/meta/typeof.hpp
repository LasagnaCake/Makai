#ifndef CTL_META_TYPEOF_H
#define CTL_META_TYPEOF_H

#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

namespace Meta {
	/// @brief Returns the type contained in the type class.
	/// @tparam T Type class.
	template<class T>
	using Unwrap = typename T::Type;

	/// @brief Returns the type contained in the type class.
	/// @tparam T Type class.
	template<class T>
	using TypeOf = Unwrap<T>;
}

CTL_NAMESPACE_END

#endif // CTL_META_LOGIC_H
