#ifndef CTL_EXTENDED_TYPES_H
#define CTL_EXTENDED_TYPES_H

#include <ctype.h>

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

/// @brief 8-bit unsigned integer.
typedef unsigned char		uint8;
/// @brief 16-bit unsigned integer.
typedef unsigned short		uint16;
/// @brief 32-bit unsigned integer.
typedef unsigned long	   	uint32;
/// @brief 64-bit unsigned integer.
typedef unsigned long long	uint64;
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

/// @brief 8-bit signed integer.
typedef signed char			int8;
/// @brief 16-bit signed integer.
typedef signed short		int16;
/// @brief 32-bit signed integer.
typedef signed long			int32;
/// @brief 64-bit signed integer.
typedef signed long long	int64;
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

/// @brief Unsigned character.
typedef uint8			uchar;
/// @brief Unsigned 16-bit integer.
typedef uint16			ushort;
/// @brief Unsigned integer.
typedef unsigned int	uint;
/// @brief Unsigned 32-bit integer.
typedef uint32			ulong;
/// @brief Unsigned 64-bit integer.
typedef uint64			ullong;

/// @brief Quadruple-precision floating point number.
typedef float128	ldouble;

/// @brief Signed character.
typedef int8	schar;
/// @brief Signed 64-bit integer.
typedef int64	llong;

/// @brief Wide character.
typedef wchar_t	wchar;

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

static_assert(sizeof(usize) == sizeof(size_t), "Size mismatch!");

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
/// @brief "C-style" wide string.
typedef wchar const*	cwstring;

/// @brief Null type.
typedef decltype(nullptr) nulltype;

#endif // CTL_EXTENDED_TYPES_H
