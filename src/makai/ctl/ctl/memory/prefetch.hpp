#ifndef CTL_MEMORY_PREFETCH_H
#define CTL_MEMORY_PREFETCH_H

#include "core.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Data prefetching facilities.
namespace Prefetch {
	/// @brief Prefetch mode.
	enum class Mode: uint8 {
		PM_READ			= 0,
		PM_WRITE		= 1,
		PM_SHARED_READ	= 2,
	};

	/// @brief Prefetch lifetime.
	enum class Lifetime: uint8 {
		PL_SINGLE_USE	= 0,
		PL_LOW			= 1,
		PL_HIGH			= 1,
		PL_PERSISTANT	= 3,
	};

	consteval void prefetch(pointer const addr, Mode const mode = Mode::PM_READ, Lifetime const time = Lifetime::PL_PERSISTANT) {
		return __builtin_prefetch(addr, static_cast<usize>(mode), static_cast<usize>(time));
	}
}


CTL_NAMESPACE_END

#endif
