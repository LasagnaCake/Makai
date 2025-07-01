#ifndef CTL_CONTAINER_DICTIONARY_H
#define CTL_CONTAINER_DICTIONARY_H

#include "../namespace.hpp"
#include "map/map.hpp"
#include "string.hpp"

CTL_NAMESPACE_BEGIN

/// @brief `Map` analog for `String`-mapped unsorted collections.
/// @tparam TValue Value type.
/// @tparam TIndex Index type.
template<class TValue, Type::Integer TIndex = usize>
using OrderedDictionary	= OrderedMap<String, TValue, TIndex>;

/// @brief `Map` analog for `String`-mapped sorted collections.
/// @tparam TValue Value type.
/// @tparam TIndex Index type.
template<class TValue, Type::Integer TIndex = usize>
using ListDictionary	= ListMap<String, TValue, TIndex>;

/// @brief `Map` analog for `String`-mapped tree-based collections.
/// @tparam TValue Value type.
/// @tparam TIndex Index type.
template<class TValue, Type::Integer TIndex = usize>
using TreeDictionary	= TreeMap<String, TValue, TIndex>;

/// @brief `Map` analog for `String`-mapped collections.
/// @tparam TValue Value type.
/// @tparam TIndex Index type.
template<class TValue, Type::Integer TIndex = usize>
using Dictionary		= TreeDictionary<TValue, TIndex>;

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_DICTIONARY_H
