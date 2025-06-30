#ifndef CTL_CONTAINER_MAP_H
#define CTL_CONTAINER_MAP_H

#include "listmap.hpp"
#include "treemap.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Analog for a key-value associative container.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TIndexOrSize Index or size type.
template<class TKey, class TValue, Type::Integer TIndexOrSize = usize>
using Map = TreeMap<TKey, TValue, TIndexOrSize>;

CTL_NAMESPACE_END

#endif