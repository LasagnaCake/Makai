#ifndef CTL_TYPETRAITS_FORCESTATE_H
#define CTL_TYPETRAITS_FORCESTATE_H

#include "../namespace.hpp"
#include "verify.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Forces the function to never be inlined.
constexpr void doNotInline() {if (inRunTime()) asm("");}

CTL_NAMESPACE_END

#endif