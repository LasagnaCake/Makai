#ifndef CTL_ASYNC_PAUSE_H
#define CTL_ASYNC_PAUSE_H

#include "../namespace.hpp"
#include "../container/functor.hpp"
#include "thread.hpp"
#include "atomic.hpp"
#include "../container/error.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Asynchronous-related errors
namespace Error::Async {
	DEFINE_ERROR_TYPE(Occupied);
}

/// @brief Asynchronous facilities.
namespace Async {
	/// @brief Tells the CPU to pause execution.
	inline void pause() {
		asm ("")
		#ifdef CTL_ON_X86
		_mm_pause();
		#else
		//asm volatile ("yield");
		asm volatile ("isb");
		#endif
	}
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_PAUSE_H
