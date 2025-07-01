#ifndef CTL_CONTAINER_TREE_COMPARATOR_H
#define CTL_CONTAINER_TREE_COMPARATOR_H

#include "../../namespace.hpp"
#include "../../cpperror.hpp"
#include "../../adapter/comparator.hpp"
#include "../../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Tree-specific type constraints.
namespace Type::Tree {
	/// @brief Whether `TComparator` forms a valid tree comparator with `TData`.
	template <class TData, template <class> class TCompare>
	concept Comparator = requires {
		{TCompare<TData>::lesser(TData(), TData())}	-> Type::Convertible<bool>;
		{TCompare<TData>::equals(TData(), TData())}	-> Type::Convertible<bool>;
	};
};

CTL_NAMESPACE_END

#endif