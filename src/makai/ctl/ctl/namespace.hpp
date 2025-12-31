#ifndef CTL_NAMESPACE_H
#define CTL_NAMESPACE_H

#ifdef CTL_DEVMODE_DEBUG
#include <iostream>
#define CTL_DEVMODE_FN_DECL			std::cout << "[" <<  __builtin_FUNCTION() << "]\n"
#define CTL_DEVMODE_FN_BEGIN		std::cout << "<" <<  __builtin_FUNCTION() << ">\n"
#define CTL_DEVMODE_FN_END			std::cout << "</" <<  __builtin_FUNCTION() << ">\n"
#define CTL_DEVMODE_OUT(CONTENT)	std::cout << (CONTENT)
#else
#define CTL_DEVMODE_FN_DECL
#define CTL_DEVMODE_FN_BEGIN
#define CTL_DEVMODE_FN_END
#define CTL_DEVMODE_OUT(CONTENT)
#endif

#define CTL_DIAGBLOCK_BEGIN _Pragma("GCC diagnostic push")
#define CTL_DIAGBLOCK_END _Pragma("GCC diagnostic pop")

#define CTL_DIAGBLOCK_IGNORE_SWITCH _Pragma("GCC diagnostic ignored \"-Wswitch\"")

/// @brief CTL core namespace.
#define CTL_NAMESPACE_BEGIN	namespace CTL {
/// @brief CTL core namespace.
#define CTL_NAMESPACE_END	}

#define CTL_OS_UNKNOWN (0)
#define CTL_OS_WINDOWS (1)
#define CTL_OS_UNIX (2)

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__) && !defined(CTL_NO_WINDOWS_PLEASE)
#define CTL_TARGET_OS (CTL_OS_WINDOWS)
#else
#define CTL_TARGET_OS (CTL_OS_UNIX)
#endif

/// @brief Core library.
namespace CTL {}

CTL_NAMESPACE_BEGIN

enum class OperatingSystem {
	OS_WINDOWS = CTL_OS_WINDOWS,
	OS_UNIX = CTL_OS_UNIX
};

/// @brief Target operating system.
constexpr auto const TARGET_OS =
	#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
	OperatingSystem::OS_WINDOWS
	#else
	OperatingSystem::OS_UNIX
	#endif
;

/// @brief Core library.
namespace CTL = ::CTL;

CTL_NAMESPACE_END

#endif // CTL_NAMESPACE_H
