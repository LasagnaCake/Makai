#ifndef CTL_EXTENDED_TYPES_H
#define CTL_EXTENDED_TYPES_H

#include <ctype.h>
#include "meta/if.hpp"

#if		INTPTR_MAX == INT64_MAX
/// @brief CPU architecture.
#define CPU_ARCH (64)
#elif	INTPTR_MAX == INT32_MAX
/// @brief CPU architecture.
#define CPU_ARCH (32)
#elif	INTPTR_MAX == INT16_MAX
/// @brief CPU architecture.
#define CPU_ARCH (16)
#else
/// @brief CPU architecture.
#define CPU_ARCH (8)
#endif

#if defined(_M_X64) || defined(__amd64__) || defined(_M_X86) || defined(__i386__)
#define X86
#endif
#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__)
#define X86
#endif

/// @brief 8-bit signed integer.
typedef signed char			int8;
/// @brief 16-bit signed integer.
typedef signed short		int16;
/// @brief 32-bit signed integer.
using int32 = CTL::Meta::If<(CTL_TARGET_OS == CTL_OS_WINDOWS), long, int>;
/// @brief 64-bit signed integer.
using int64 = CTL::Meta::If<(CTL_TARGET_OS == CTL_OS_WINDOWS), long long, long>;
#if		CPU_ARCH == 64
/// @brief Signed integer of maximum architecture size.
typedef	int64				intmax;
#elif	CPU_ARCH == 32
/// @brief Signed integer of maximum architecture size.
typedef	int32				intmax;
#elif	CPU_ARCH == 16
/// @brief Signed integer of maximum architecture size.
typedef	int16				intmax;
#else
/// @brief Signed integer of maximum architecture size.
typedef	int8				intmax;
#endif

/// @brief 8-bit unsigned integer.
typedef unsigned char		uint8;
/// @brief 16-bit unsigned integer.
typedef unsigned short		uint16;
/// @brief 32-bit unsigned integer.
using uint32 = CTL::Meta::If<(CTL_TARGET_OS == CTL_OS_WINDOWS), unsigned long, unsigned int>;
/// @brief 64-bit unsigned integer.
using uint64 = CTL::Meta::If<(CTL_TARGET_OS == CTL_OS_WINDOWS), unsigned long long, unsigned long>;
#if		CPU_ARCH == 64
/// @brief Unsigned integer of maximum architecture size.
typedef	uint64				uintmax;
#elif	CPU_ARCH == 32
/// @brief Unsigned integer of maximum architecture size.
typedef	uint32				uintmax;
#elif	CPU_ARCH == 16
/// @brief Unsigned integer of maximum architecture size.
typedef	uint16				uintmax;
#else
/// @brief Unsigned integer of maximum architecture size.
typedef	uint8				uintmax;
#endif

/// @brief 32-bit floating point number.
typedef float				float32;
/// @brief 64-bit floating point number.
typedef double				float64;
/// @brief 128-bit floating point number.
typedef long double			float128;
#if		CPU_ARCH == 64
/// @brief Floating point number of maximum architecture size.
typedef	float64				floatmax;
#else
/// @brief Floating point number of maximum architecture size.
typedef	float32				floatmax;
#endif

/// @brief Quadruple-precision floating point number.
typedef float128	ldouble;

/// @brief Wide character.
typedef wchar_t	wchar;
/// @brief UTF-8 character.
typedef char8_t u8char;

/// @brief Signed byte.
typedef int8	sbyte;
/// @brief Unsigned byte.
typedef uint8	ubyte;
/// @brief Signed word.
typedef int16	sword;
/// @brief Unsigned word.
typedef uint16	uword;
/// @brief Unsigned dobule-length word.
typedef uint32	udword;
/// @brief Unsigned quadruple-length word.
typedef uint64	uqword;

/// @brief Byte.
typedef ubyte	byte;
/// @brief Word.
typedef uword	word;
/// @brief Double-length word.
typedef udword	dword;
/// @brief Quadruple-length word.
typedef uqword	qword;

#if CPU_ARCH == 64
/// @brief Unsigned size type.
typedef uint64	usize;
/// @brief Signed size type.
typedef int64	ssize;
#elif CPU_ARCH == 32
/// @brief Unsigned size type.
typedef uint32	usize;
/// @brief Signed size type.
typedef int32	ssize;
#elif CPU_ARCH == 16
/// @brief Unsigned size type.
typedef uint16	usize;
/// @brief Signed size type.
typedef int16	ssize;
#elif CPU_ARCH == 8
/// @brief Unsigned size type.
typedef uint8	usize;
/// @brief Signed size type.
typedef int8	ssize;
#endif

typedef uint64 litsize;

typedef unsigned long long	litint;
typedef long double			litfloat;

typedef unsigned int uint;

#if CPU_ARCH == 64
/// @brief Signed 128-bit integer.
typedef __int128_t	int128;

/// @brief Unsigned 128-bit integer.
typedef __uint128_t	uint128;
#endif

/// @brief Generic pointer.
typedef void*	pointer;

/// @brief "C-style" string.
typedef char const*		cstring;
/// @brief "C-style" UTF-8 string.
typedef u8char const*	u8cstring;
/// @brief "C-style" wide string.
typedef wchar const*	cwstring;

/// @brief Null type.
typedef decltype(nullptr) nulltype;

/// @brief Type identity. Decays to `T`.
/// @tparam T Type.
/// @note Used so certain types, like const references to C-style arrays, become valid declarations.
/// @example As<int[32]> const& a; // Instead of: int (const& a)[32].
template<class T>
using As = T;

/// @brief Syntatic sugar for `T*`.
/// @tparam T Pointed type.
template<class T>
using ptr = T*;

/// @brief
///		Syntatic sugar for `T*`.
///		Hints to the programmer that the value is owned.
///
///		- If field/parameter, then it takes ownership of the data pointed to.
///
///		- If return value, then the caller takes ownership of the data pointed to.
/// @tparam T Pointed type.
template<class T>
using owner = ptr<T>;

/// @brief
///		Syntatic sugar for `T*`.
///		Hints to the programmer that the value is not owned.
///
///		- If field/parameter, then it does not take ownership of the data pointed to.
///
///		- If return value, then the caller does not take ownership of the data pointed to.
/// @tparam T Pointed type.
template<class T>
using ref = ptr<T>;

/// @brief Syntatic sugar for `T const&`. Indicates a variable is a read-only reference.
template<class T> using in		= T const&;
/// @brief Syntatic sugar for `T&`. Indicates a variable is a reference that can be read from, and written to.
template<class T> using inout	= T&;

/// @brief Null value. Syntatic sugar for `nullptr`.
constexpr nulltype const null{};

#endif // CTL_EXTENDED_TYPES_H
