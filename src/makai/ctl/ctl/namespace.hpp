#ifndef CTL_NAMESPACE_H
#define CTL_NAMESPACE_H

#ifdef CTL_DEVMODE_DEBUG
#include <iostream>
#define CTL_DEVMODE_FN_BEGIN		std::cout << "<" <<  __builtin_FUNCTION() << ">\n"
#define CTL_DEVMODE_FN_END			std::cout << "</" <<  __builtin_FUNCTION() << ">\n"
#define CTL_DEVMODE_OUT(CONTENT)	std::cout << CONTENT
namespace CTL::_Devmode {
	struct Scope {
		const char* const fname;
		Scope(const char* fname): fname(fname) {CTL_DEVMODE_OUT("<" <<  fname << ">\n");}
		~Scope() {CTL_DEVMODE_OUT("</" << fname << ">\n");}
	};
}
#define CTL_DEVMODE_FN_DECL			auto const _SCOPE = ::CTL::_Devmode::Scope(__builtin_FUNCTION())
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
#define CTL_ON_WINDOWS (1)
#else
#define CTL_TARGET_OS (CTL_OS_UNIX)
#define CTL_ON_UNIX (1)
#endif

#define CTL_CXX_UNKNOWN (0)
#define CTL_CXX_GCC (1)
#define CTL_CXX_CLANG (2)
#define CTL_CXX_MSVC (2)

#ifdef __clang__
#define CTL_TARGET_COMPILER (CTL_CXX_CLANG)
#define CTL_ON_CLANG (1)
#else
#ifdef __GNUC__
#define CTL_TARGET_COMPILER (CTL_CXX_GCC)
#define CTL_ON_GCC (1)
#else
#define CTL_TARGET_COMPILER (CTL_CXX_MCTL_CXX_MSVC)
#define CTL_ON_MSVC (1)
#endif
#endif

#if CTL_ON_WINDOWS
#define CTL_DYNEXPORT __declspec(dllexport)
#define CTL_DYNIMPORT __declspec(dllimport)
#elif CTL_ON_UNIX
#define CTL_DYNEXPORT __attribute__((visibility("default")))
#define CTL_DYNIMPORT
#else
#define CTL_DYNEXPORT
#define CTL_DYNIMPORT
#pragma warning "What system is this?"
#endif

#define CTL_DO_NOT_INLINE asm("")

#ifdef CTL_BUILD_MODE
#define CTL_DYNCALL CTL_DYNEXPORT
#else
#define CTL_DYNCALL CTL_DYNIMPORT
#endif

#define CTL_CDECL extern "C"

#ifdef CTL_ON_CLANG
#define CTL_UNAVAILABLE(REASON) clang::unavailable(REASON)
#else
#define CTL_UNAVAILABLE(REASON)  gnu::unavailable(REASON)
#endif

/// @brief Core library.
namespace CTL {
}

CTL_NAMESPACE_BEGIN

enum class OperatingSystem {
	OS_WINDOWS = CTL_OS_WINDOWS,
	OS_UNIX = CTL_OS_UNIX
};

enum class Compiler {
	CXX_GCC = CTL_CXX_GCC,
	CXX_CLANG = CTL_CXX_CLANG,
	CXX_MSVC = CTL_CXX_MSVC
};

/// @brief Target operating system.
constexpr auto const TARGET_OS =
	#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
	OperatingSystem::OS_WINDOWS
	#else
	OperatingSystem::OS_UNIX
	#endif
;

/// @brief Target compiler.
constexpr auto const TARGET_CXX =
	#if (CTL_TARGET_COMPILER == CTL_CXX_GCC)
	Compiler::CXX_GCC
	#else
	#if (CTL_TARGET_COMPILER == CTL_CXX_CLANG)
	Compiler::CXX_CLANG
	#else
	Compiler::CXX_MSVC
	#endif
	#endif
;

/// @brief Core library.
namespace CTL = ::CTL;

CTL_NAMESPACE_END

#ifdef CTL_ON_MSVC
#error "MSVC is not supported!"
#endif

#endif // CTL_NAMESPACE_H
