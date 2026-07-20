#ifndef CTL_CONTAINER_SYNC_COW_H
#define CTL_CONTAINER_SYNC_COW_H

#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../typeinfo.hpp"
#include "../../ctypes.hpp"
#include "../../typetraits/traits.hpp"
#include "../../async/lock.hpp"
#include "../pointer/atomiccell.hpp"


CTL_NAMESPACE_BEGIN

/// @brief Synchronization facilities.
namespace Sync {

/// @brief Copy-on-Write value.
/// @tparam TData Value type.
template <class TData>
struct Cow {

};

}

CTL_NAMESPACE_END

#endif
