#ifndef CTL_ALGORITHM_ATOI_H
#define CTL_ALGORITHM_ATOI_H

#include "validate.hpp"
#include "transform.hpp"
#include "../memory/memory.hpp"
#include "../cmath.hpp"
#include "../namespace.hpp"
#include "../typetraits/decay.hpp"

CTL_NAMESPACE_BEGIN

// atoi implementation based off of https://stackoverflow.com/a/59537554
/// @brief General implementations.
namespace Impl {
	/// @brief Array-to-integer implementation.
	namespace A2I {
		/// @brief Converts a character to number.
		/// @tparam T Character type.
		/// @param c Character to convert.
		/// @return Character as number.
		template<Type::ASCII T>
		constexpr ssize toDigit(T c) {
			c = toLowerChar(c);
			if (c >= 'a')
				return c - 'a' + 10;
			return c - '0';
		}

		/// @brief Returns whether the given character is in the specified base.
		/// @tparam T Character type.
		/// @param c Character to check.
		/// @param base Base to check against.
		/// @return Whether it is in the given base.
		template<Type::ASCII T>
		constexpr bool isDigitInBase(T const& c, usize const base) {
			if (!base) return true;
			if (c == '.') return true;
			ssize const v = toDigit(c);
			return 0 <= v && v < ssize(base);
		}

		/// @brief Returns whether the given character is a numeric sign (`+` or `-`).
		/// @tparam T Character type.
		/// @param c Character to check.
		/// @return Whether the character is a numeric sign.
		template<Type::ASCII T>
		constexpr bool isSign(T const& c) {
			return c == '-' || c == '+';
		}

		/// @brief Returns the signedness of the character at the given pointer, then increments said pointer.
		/// @tparam T Character type.
		/// @param c Pointer to character to check.
		/// @return Signedness of character. -1 if character is `-`, +1 otherwise.
		/// @note Only advances the pointer if character is a numeric sign.
		template<Type::ASCII T>
		constexpr int8 getSignAndConsume(ref<T const>& c) {
			switch (c[0]) {
				case '-':	++c; return -1;
				case '+':	++c;
				default:	return +1;
			}
		}

		/// @brief Shifts a value by a given base, and appends another value to it.
		/// @tparam T Integer type.
		/// @param val Target to append to.
		/// @param base Base to shift by.
		/// @param digit Value to append.
		/// @return Reference to target.
		template<Type::Integer T>
		constexpr T& shiftAndAppend(T& val, T const& base, T const& digit) {
			val *= base;
			val += digit;
			return val;
		}

		/// @brief Returns whether the given character is a (possibe) valid integer character.
		/// @tparam T Character type.
		/// @param c Character to check.
		/// @return Whether it is a valid integer character.
		template<Type::ASCII T>
		constexpr bool isInvalidChar(T const c) {
			//return isNullOrSpaceChar(c) || c == '.' || c <= ' ';
			return !isAlphanumericChar(c);
		}

		/// @brief Returns the size of the integer stored in the string.
		/// @tparam T Character type.
		/// @param str String to get integer size.
		/// @param size Size of string.
		/// @return Stored integer size.
		template<Type::ASCII T>
		constexpr usize integerSize(ref<T const> str, usize const size) {
			usize result = 0;
			while (result < size) {
				if (isInvalidChar(str[result])) break;
				++result;
			}
			return result;
		}

		/// @brief Converts a string of charaters to an integer.
		/// @tparam I Integer type.
		/// @tparam T Character type.
		/// @param str String to convert.
		/// @param size Size of string.
		/// @param base Base of number.
		/// @return Resulting integer in the given base.
		template<Type::Integer I, Type::ASCII T>
		constexpr I toInteger(ref<T const> str, usize const size, usize const base) {
			I res = 0;
			for (usize i = 0; i < size; ++i) {
				if (isInvalidChar(str[i])) break;
				shiftAndAppend<I>(res, base, toDigit(str[i]));
			}
			return res;
		}

		/// @brief Returns the base of the character at the given pointer, then increments said pointer.
		/// @tparam T Character type.
		/// @param c Pointer to character to check.
		/// @param base Base override. Will be returned instead if it is not zero.
		/// @return Base of the character identifier. If no match is found, returns 10.
		template<Type::ASCII T>
		constexpr ssize getBaseAndConsume(ref<T const>& c, usize const base) {
			if (c[0] == '0') {
				++c;
				switch (c[0]) {
					case 'y':	++c; return base ? base : 32;
					case 'x':	++c; return base ? base : 16;
					case 'q':	++c; return base ? base : 4;
					case 't':	++c; return base ? base : 3;
					case 'b':	++c; return base ? base : 2;
					case 'd':	++c; return base ? base : 10;
					case 'o':	++c; return base ? base : 8;
					default: break;
				}
			}
			return base ? base : 10;
		}

		/// @brief Returns whether the given string is in the specified base.
		/// @tparam T Character type.
		/// @param str string to check.
		/// @param size Size of string to check.
		/// @param base Base to check against.
		/// @return Whether it is in the given base.
		template<Type::ASCII T>
		constexpr bool isInBase(ref<T const> str, usize const size, usize const base) {
			if (!base) return true;
			for (usize i = 0; i < integerSize(str, size); ++i) {
				if (isInvalidChar(str[i]))
					break;
				else if (!isDigitInBase(str[i], base))
					return false;
			}
			return true;
		}
	}
}

/// @brief Converts a string of characters to an integer.
/// @tparam I Integer type.
/// @tparam T Character type.
/// @param str String to convert.
/// @param size Size of string to convert.
/// @param out Output of the conversion.
/// @param base Base to convert from. If zero, then base is deduced. By default, it is zero.
/// @return Whether the operation was successful.
/// @note
///		Valid base prefixes:
///
///		- `0b`:	Binary.
///
///		- `0t`:	Trinary.
///
///		- `0q`:	Quaternary.
///
///		- `0o`:	Octal.
///
///		- `0d`:	Decimal.
///
///		- `0x`:	Hexadecimal.
///
///		- `0y`:	Duotrigesimal.
/// @warning "Implicit Octals" (octal values whose prefix are only `0`) are invalid, and are treated as decimal values!
template<Type::Integer I, Type::ASCII T>
constexpr bool atoi(ref<T const> const str, usize size, I& out, usize base = 0) {
	// Copy string pointer
	ref<T const> s = str;
	// If string is size 1, try and convert digit
	if (size == 1) {
		if (Impl::A2I::isDigitInBase(str[0], base) || base == 0) {
			out = Impl::A2I::toDigit(str[0]);
			return true;
		}
		else return false;
	}
	// If size is 2, first character is a sign, and is in base, convert number to digit
	if (
		size == 2
	&&	Impl::A2I::isSign(str[0])
	&&	(
		Impl::A2I::isDigitInBase(str[1], base) || base == 0
	)) {
		out = Impl::A2I::getSignAndConsume<T>(s) * Impl::A2I::toDigit<T>(s[0]);
		return true;
	}
	// Try and get sign of number
	ssize sign = Impl::A2I::getSignAndConsume(s);
	if (ssize(size) < s-str) return false;
	// Try and get base of number
	base = Impl::A2I::getBaseAndConsume(s, base);
	if (ssize(size) < s-str) return false;
	// Remove difference from size
	size -= s-str;
	// Check if number is in its intended base
	if (!Impl::A2I::isInBase(s, size, base))
		return false;
	// Convert number to int and return
	out = sign * Impl::A2I::toInteger<I>(s, size, base);
	return true;
}

/// @brief Converts a fixed array of characters to an integer.
/// @tparam I Integer type.
/// @tparam T Character type.
/// @tparam S Array size.
/// @param str String to convert.
/// @param out Output of the conversion.
/// @param base Base to convert from. If zero, then base is deduced. By default, it is zero.
/// @return Whether the operation was successful.
/// @note
///		Valid base prefixes:
///
///		- `0b`:	Binary.
///
///		- `0t`:	Trinary.
///
///		- `0q`:	Quaternary.
///
///		- `0o`:	Octal.
///
///		- `0d`:	Decimal.
///
///		- `0x`:	Hexadecimal.
///
///		- `0y`:	Duotrigesimal.
/// @warning "Implicit Octals" (octal values whose prefix are only `0`) are invalid, and are treated as decimal values!
template<Type::Integer I, Type::ASCII T, usize S>
constexpr bool atoi(As<const T[S]> const& str, I& out, usize const base = 0) {
	static_assert(S-1 > 0, "String cannot be empty!");
	return ::CTL::atoi<I, T>(str, S - 1, out, base);
}

/// @brief Converts a string of characters into a floating point number.
/// @tparam F Floating point type.
/// @tparam T Character type.
/// @param str String to convert.
/// @param size Size of string to convert.
/// @param out Output of the conversion.
/// @param base Base to convert from. If zero, then base is deduced. By default, it is zero.
/// @return Whether the operation was successful.
/// @note Same base prefixes as `atoi`.
/// @warning "Implicit Octals" (octal values whose prefix are only `0`) are invalid, and are treated as decimal values!
template<Type::Real F, Type::ASCII T>
constexpr bool atof(ref<T const> const str, usize size, F& out, usize const base = 0) {
	// If character is appended to the end, exclude it
	if (
		toLowerChar(str[size-1]) == 'f'
	||	toLowerChar(str[size-1]) == 'd'
	) --size;
	// Find separator character
	usize sep = 0;
	while (sep < size)
		if (str[sep++] == '.')
			break;
	// If no separator was found, convert number and return
	ssize ival = 0;
	if (sep == size) {
		if (!atoi<ssize>(str, size, ival, base))
			return false;
		out = ival;
		return true;
	}
	// Create new string without separator
	T* ns = new T[size-1];
	MX::memmove(ns, str, sep-1);
	MX::memmove(ns+sep-1, str+sep, size-sep);
	// Try and convert resulting integer string
	if (!::CTL::atoi<ssize>(ns, size-1, ival, base))
		return false;
	delete[] ns;
	// Convert integer to string by "reverse scientific notation" and return
	out = ival * Math::pow<F>(10, -ssize(size-sep));
	return true;
}

/// @brief Converts a fixed array of characters into a floating point number.
/// @tparam F Floating point type.
/// @tparam T Character type.
/// @tparam S Array size.
/// @param str String to convert.
/// @param out Output of the conversion.
/// @param base Base to convert from. If zero, then base is deduced. By default, it is zero.
/// @return Whether the operation was successful.
/// @note Same base prefixes as `atoi`.
/// @warning "Implicit Octals" (octal values whose prefix are only `0`) are invalid, and are treated as decimal values!
template<Type::Real F, Type::ASCII T, usize S>
constexpr bool atof(As<const T[S]> const& str, F& out, usize const base = 0) {
	static_assert(S-1 > 0, "String cannot be empty!");
	return ::CTL::atof<F, T>(str, S - 1, out);
}

// Based off of https://stackoverflow.com/a/3987783

/// @brief Converts an integer into a string of characters.
/// @tparam I Integer type.
/// @tparam T Character type.
/// @param val Integer to convert.
/// @param buf Output string buffer of the conversion.
/// @param bufSize String buffer size.
/// @param base Base to convert to. By default, it is base 10.
/// @return Size of resulting number string.
template<Type::Integer I, Type::ASCII T>
constexpr ssize itoa(I val, ref<T> const buf, usize const bufSize, I const& base = 10) {
	// Digits
	cstring const digits = "0123456789abcdefghijklmnopqrstuv";
	// If empty buffer, or buffer is too small for a non-decimal base
	if ((!bufSize) || (bufSize < 4 && base != 10))
		return -1;
	// Clear buffer
	MX::memzero(buf, bufSize);
	// Get stating points
	usize
		offset = 0,
		i = bufSize-2
	;
	// If value is 0, set buffer to zero and return
	if (!val) {
		buf[0] = '0';
		return 1;
	}
	// If value is negative, append negative sign and invert value
	if (val < 0) {
		buf[offset++] = '-';
		val = -val;
	}
	// If not decimal, append base identifier accoordingly
	if (base != 10) {
		buf[offset++] = '0';
		switch (base) {
			case 2:		buf[offset] = 'b'; ++offset; break;
			case 3:		buf[offset] = 't'; ++offset; break;
			case 4:		buf[offset] = 'q'; ++offset; break;
			case 8:		buf[offset] = 'o'; ++offset; break;
			case 16:	buf[offset] = 'x'; ++offset; break;
			case 32:	buf[offset] = 'y'; ++offset; break;
			default: break;
		}
	}
	// Calculate number value
	for(; val && (i-offset); --i, val /= base) {
		buf[i] = digits[val % base];
	}
	// Move stuff around to beginning of buffer
	MX::memmove(buf+offset, buf+offset+i, bufSize-i);
	if (!offset)
		MX::memmove(buf, buf+1, bufSize-i);
	// Return full size of number string
	return (bufSize - i - 2 + offset);
}

/// @brief Converts a floating point number into a string of characters.
/// @tparam F Floating point type.
/// @tparam T Character type.
/// @param val Floating point number to convert.
/// @param buf Output string buffer of the conversion.
/// @param bufSize String buffer size.
/// @param
///		precision Amount of decimal spaces to include.
///		By default, it is equal to double the byte size of the floating point type.
/// @return Size of resulting number string.
/// @note
///		Default value of `precision` for:
///
///		- `float`s: 8 decimal spaces.
///
///		- `double`s: 16 decimal spaces.
///
///		- `long double`s: 32 decimal spaces.
template<Type::Real F, Type::ASCII T>
constexpr ssize ftoa(F val, ref<T> buf, usize bufSize, usize const precision = sizeof(F)*2) {
	// Get amount of zeroes to add to number
	usize zeroes = Math::pow<F>(10, precision);
	// If value is negative, append negative sign and invert value
	if (val < 0) {
		*(buf++) = '-';
		val = -val;
		--bufSize;
	}
	// Get whole part of number
	ssize whole = val;
	// Get fractional part
	usize frac = ((val - whole) * zeroes + 0.49);
	ssize lhs = 0, rhs = 0;
	// Fill in whole part of number to string, return if error
	if ((lhs = ::CTL::itoa<ssize>(whole, buf, bufSize)) == -1) return -1;
	// Check if buffer is not full, else append comma and re-check
	if (usize(lhs) >= bufSize) return lhs;
	buf[lhs++] = '.';
	if (usize(lhs) >= bufSize) return lhs;
	// Fill in fractional part, returning if error
	if ((rhs = ::CTL::itoa<ssize>(frac, buf+lhs, bufSize-lhs)) == -1) return -1;
	// Return full size of number string
	return lhs+rhs+1;
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_ATOI_H
