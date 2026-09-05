#ifndef CTL_META_SELF_H
#define CTL_META_SELF_H

#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Self-referential class.
template <class T>
struct Self {
	constexpr T& self()				{return *Cast::as<ref<T>>(this);		}
	constexpr T const& self() const	{return *Cast::as<ref<T const>>(this);	}
};

CTL_NAMESPACE_END

#endif
