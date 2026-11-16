#ifndef CTL_CPP_DEBUG_H
#define CTL_CPP_DEBUG_H

#include "../namespace.hpp"
#include "../algorithm/cstrop.hpp"
#include <signal.h>

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <debugapi.h>
#elif (CTL_TARGET_OS == CTL_OS_UNIX)
#include <cstdio>
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
		#elif (CTL_TARGET_OS == CTL_OS_UNIX)
		auto const cf = fopen("/proc/self/status", "r");
		if (!cf) return false;
		As<char[4096]> pinfo;
		auto const count = fread(pinfo, 1, 4096, cf);
		if (ferror(cf)) {
			fclose(cf);
			return false;
		}
		pinfo[count < 4096 ? count : 4095] = '\0';
		constexpr auto tracer = "TracerPid:\t";
		const auto tp = strstr(pinfo, count, tracer, sizeof(tracer));
		if (!tp) return false;
		for (cstring cc = tp + 11; cc < (pinfo + count); ++cc) {
			if (*cc <= ' ') continue;
			else {
				fclose(cf);
				return (*cc > '0' && *cc < '9');
			}
		}
		fclose(cf);
		return false;
		#else
		return false;
		#endif
	}

	/// @brief Detects whether or not the program is running on a debugger.
	/// @return Whether debugger is present.
	inline bool debugged() {
		return hasDebugger();
	}

	/// @brief Traceable function. Can be used in debugging.
	[[gnu::noinline, maybe_unused]] static void trace()			{CTL_DO_NOT_INLINE;								}
	/// @brief Emits a breakpoint.
	[[gnu::noinline, maybe_unused]] static void breakpoint()	{CTL_DO_NOT_INLINE; fire();						}
	/// @brief Emits a breakpoint.
	[[gnu::noinline, maybe_unused]] static void dbreak()		{CTL_DO_NOT_INLINE; if (hasDebugger()) fire();	}

	/// @brief Traceable object.
	struct Traceable {
		/// @brief Constructor.
		inline Traceable() {if (trap) breakpoint(); else trace();}

		inline static bool trap = false;
	};
}

CTL_NAMESPACE_END

#endif
