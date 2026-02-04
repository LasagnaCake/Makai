#ifndef CTL_ALGORITHM_STRCONV_H
#define CTL_ALGORITHM_STRCONV_H

#include "../namespace.hpp"
#include "../container/strings/strings.hpp"

CTL_NAMESPACE_BEGIN

/// @brief String-to-`int` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr int		toInt(String const& s, uint8 const base = 0)				{return String::toNumber<int>(s, base);			}
/// @brief String-to-`long` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr long		toLong(String const& s, uint8 const base = 0)				{return String::toNumber<long>(s, base);		}
/// @brief String-to-`long long` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr long long	toLongLong(String const& s, uint8 const base = 0)			{return String::toNumber<long long>(s, base);	}

/// @brief String-to-`unsigned int` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr unsigned int			toUnsignedInt(String const& s, uint8 const base = 0)		{return String::toNumber<uint>(s, base);	}
/// @brief String-to-`unsigned long` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr unsigned long			toUnsignedLong(String const& s, uint8 const base = 0)		{return String::toNumber<ulong>(s, base);	}
/// @brief String-to-`unsigned long long` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr unsigned long long	toUnsignedLongLong(String const& s, uint8 const base = 0)	{return String::toNumber<litint>(s, base);	}

/// @brief String-to-`float` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr float		toFloat(String const& s, usize const base = 0)		{return String::toNumber<float>(s, base);	}
/// @brief String-to-`double` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr double	toDouble(String const& s, usize const base = 0)		{return String::toNumber<double>(s, base);	}
/// @brief String-to-`long double` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
constexpr ldouble	toLongDouble(String const& s, usize const base = 0)	{return String::toNumber<ldouble>(s, base);	}

/// @brief String-to-`int8` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline int8		toInt8(String const& s, uint8 const base = 0)		{return String::toNumber<int8>(s, base);	}
/// @brief String-to-`int16` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline int16	toInt16(String const& s, uint8 const base = 0)		{return String::toNumber<int16>(s, base);	}
/// @brief String-to-`int32` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline int32	toInt32(String const& s, uint8 const base = 0)		{return String::toNumber<int32>(s, base);	}
/// @brief String-to-`int64` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline int64	toInt64(String const& s, uint8 const base = 0)		{return String::toNumber<int64>(s, base);	}

/// @brief String-to-`uint8` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline uint8	toUInt8(String const& s, uint8 const base = 0)		{return String::toNumber<uint8>(s, base);	}
/// @brief String-to-`uint16` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline uint16	toUInt16(String const& s, uint16 const base = 0)	{return String::toNumber<uint16>(s, base);	}
/// @brief String-to-`uint32` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline uint32	toUInt32(String const& s, uint32 const base = 0)	{return String::toNumber<uint32>(s, base);	}
/// @brief String-to-`uint64` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline uint64	toUInt64(String const& s, uint64 const base = 0)	{return String::toNumber<uint64>(s, base);	}

/// @brief String-to-`float32` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline float32	toFloat32(String const& s, usize const base = 0)	{return toFloat(s, base);		}
/// @brief String-to-`float64` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline float64	toFloat64(String const& s, usize const base = 0)	{return toDouble(s, base);		}
/// @brief String-to-`float128` conversion.
/// @param s String to convert.
/// @param base Base of number string. Will be used, if non-zero.
/// @return Resulting number.
/// @throw FailedActionException if conversion fails.
inline float128	toFloat128(String const& s, usize const base = 0)	{return toLongDouble(s, base);	}

/// @brief String-to-`bool` conversion.
/// @param s String to convert.
/// @return Resulting value.
/// @throw FailedActionException if conversion fails.
inline float128	toBool(String const& s)	{return String::toNumber<bool>(s);	}

/// @brief Number-to-string conversion.
/// @tparam T Number type.
/// @param value Value to convert.
/// @return Resulting string.
template<Type::Number T>
constexpr String toString(T const& value)
requires (!(Type::Equal<T, bool> || Type::ASCII<T>)) {
	return String::fromNumber<T>(value);
}

/// @brief Character-to-string conversion.
/// @tparam T Character type (`char` or `wchar`).
/// @param value Value to convert.
/// @return Resulting string.
template<Type::Equal<char> T>
constexpr String toString(T const& value) {
	return String() + value;
}

/// @brief Boolean-to-string conversion.
/// @tparam T Boolean type (`bool`).
/// @param value Value to convert.
/// @return Resulting string.
template<Type::Equal<bool> T>
constexpr String toString(T const& value, bool const text = false) {
	return String::fromNumber<bool>(value, text);
}

/// @brief Conversion-specific type traits.
namespace Type::Conversion {
	/// @brief Class must have a `toString()` function.
	template<class T>
	concept ClassStringable = requires (T t) {
		{t.toString()} -> Type::Equal<String>;
	};
}

/// @brief Class-to-string conversion.
/// @tparam T Class type.
/// @param value Value to convert.
/// @return Resulting string.
/// @note Requires type to be implicitly convertible to string, and not possess a `toString` function.
template<Type::CanBecome<String> T>
constexpr String toString(T const& value)
requires (
	Type::Class<T>
&& !Type::Conversion::ClassStringable<T>
) {
	return String(value);
}

/// @brief Class-to-string conversion.
/// @tparam T Class type.
/// @param value Value to convert.
/// @return Resulting string.
/// @note Requires type to have a `toString()` function.
template<Type::Conversion::ClassStringable T>
constexpr String toString(T const& value) {
	return value.toString();
}

namespace Type::Conversion {
	/// @brief Type must be convertible to a "C-style" string via a `c_str()` function.
	template<class T>
	concept STDStringable = requires (T t) {
		{t.c_str()} -> Type::Equal<typename String::CStringType>;
	};
}

/// @brief Class-to-string conversion.
/// @tparam T Class type.
/// @param value Value to convert.
/// @return Resulting string.
/// @note Requires type to be convertible to a "C-style" string via a `c_str()` function.
template<Type::Conversion::STDStringable T>
constexpr String toString(T const& value) {
	return value.c_str();
}

namespace Type::Conversion {
	/// @brief Type must be convertible to string in some form.
	template<class T>
	concept StringConvertible = requires (T t) {
		{toString(t)} -> Type::Equal<String>;
	};
}

/// @brief null-terminated string conversion.
/// @param value String to convert.
/// @return Resulting string.
constexpr String toString(cstring const& value)						{return String(value);			}
/// @brief "C-style" string conversion.
/// @param value String to convert.
/// @param size Size of string to convert.
/// @return Resulting string.
constexpr String toString(cstring const& value, usize const size)	{return String(value, size);	}
/// @brief string-to-string conversion (returns passed string).
/// @param value String to convert.
/// @return Passed string.
constexpr String toString(String const& value)						{return value;					}

/// @brief Char array conversion.
/// @tparam T Character type.
/// @tparam N Array size.
/// @param value Character array to convert.
/// @return Resulting string.
template<Type::ASCII T, usize N>
constexpr String toString(As<T[N]> const& value) {
	return String(value);
}

/// @brief String conversion with multiple values.
/// @tparam ...Args Argument types.
/// @param ...args Values to convert.
/// @return All values as strings, concatenated sequentially into a single string.
template<class... Args>
constexpr String toString(Args const&... args)
requires (sizeof...(Args) > 1) {
	return (... + toString(args));
}

/// @brief Wide string conversion with multiple values.
/// @tparam ...Args Argument types.
/// @param ...args Values to convert.
/// @return All values as strings, concatenated sequentially into a single wide string.
template<class... Args>
constexpr WideString toWideString(Args const&... args)
requires (... && Type::Conversion::StringConvertible<Args>) {
	return toString(args...).toWideString();
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_STRCONV_H
