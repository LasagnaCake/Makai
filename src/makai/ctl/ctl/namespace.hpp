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

/// @brief Core library.
namespace CTL {}

CTL_NAMESPACE_BEGIN

/// @brief Core library.
namespace CTL = ::CTL;

CTL_NAMESPACE_END

#endif // CTL_NAMESPACE_H
