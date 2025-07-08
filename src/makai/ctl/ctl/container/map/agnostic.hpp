#ifndef CTL_CONTAINER_MAP_AGNOSTIC_H
#define CTL_CONTAINER_MAP_AGNOSTIC_H

#include "listmap.hpp"
#include "treemap.hpp"
#include "hashmap.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Analog for a key-value associative container.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TIndexOrSize Index or size type.
template<class TKey, class TValue, Type::Integer TIndexOrSize = usize>
using Map = ListMap<TKey, TValue, TIndexOrSize>;

/// @brief Tags the deriving class as a collection of key-value pairs.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TPair<class, class> Pair type.
template<
	class TKey,
	class TValue,
	template <class, class> class TPair	= Pair
>
using Collected = TreeCollected<TKey, TValue, TPair>;

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Type MUST be one of the available associative containers.
	template<class T>
	concept Map = ListMap<T> || TreeMap<T> || HashMap<T>;
}

CTL_NAMESPACE_END

#endif