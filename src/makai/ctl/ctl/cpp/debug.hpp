#ifndef CTL_CPP_DEBUG_H
#define CTL_CPP_DEBUG_H

#include "../namespace.hpp"
#include <signal.h>

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <debugapi.h>
#else
#endif

CTL_NAMESPACE_BEGIN

/// @brief Debug facilities.
namespace CPP::Debug {
	namespace {
		inline void fire() {
			#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
			__debugbreak();
			#else
			asm("int3");
			#endif
		}
	}

	/// @brief Detects whether or not the program is running on a debugger.
	/// @return Whether debugger is present.
	inline bool hasDebugger() {
		#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
		return IsDebuggerPresent();
		#else
		return false;
		#endif
	}

	/// @brief Traceable function. Can be used in debugging.
	[[gnu::noinline, maybe_unused]] static void trace()			{			}
	/// @brief Emits a breakpoint.
	[[gnu::noinline, maybe_unused]] static void breakpoint()	{fire();	}

	/// @brief Traceable object.
	struct Traceable {
		/// @brief Constructor.
		inline Traceable() {if (trap) breakpoint(); else trace();}

		inline static bool trap = false;
	};
}

CTL_NAMESPACE_END

#endif