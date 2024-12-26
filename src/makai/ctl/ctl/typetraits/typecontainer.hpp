#ifndef CTL_TYPETRAITS_TYPECONTAINER_H
#define CTL_TYPETRAITS_TYPECONTAINER_H

#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Contains a singe `Type` of type `T`. Used in type conversions.
template<class T> struct TypeContainer {using Type = T;};

CTL_NAMESPACE_END

#endif // CTL_TYPETRAITS_TYPECONTAINER_H
