#ifndef CTL_TYPETRAITS_VERIFY_H
#define CTL_TYPETRAITS_VERIFY_H

#include "basictraits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Returns whether the current function is running in compile-time.
/// @return Whether current function is in compile-time.
consteval bool inCompileTime() noexcept {
	return __builtin_is_constant_evaluated();
}

/// @brief Returns whether the current function is running in run-time.
/// @return Whether current function is in run-time.
consteval bool inRunTime() noexcept {
	return !inCompileTime();
}

/// @brief Declares a code path as unreachable.
[[noreturn, gnu::always_inline]]
inline void unreachable() {
	__builtin_unreachable();
}

CTL_NAMESPACE_END

#endif