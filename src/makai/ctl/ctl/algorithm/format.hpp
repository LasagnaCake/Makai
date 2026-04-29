#ifndef CTL_ALGORITHM_FORMAT_H
#define CTL_ALGORITHM_FORMAT_H

#include "../namespace.hpp"
#include "../container/strings/strings.hpp"
#include "../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

namespace Format {
	/// @brief Text justification direction.
	enum class Justify {
		CFJ_LEFT,
		CFJ_RIGHT,
		CFJ_CENTER
	};

	/// @brief Pads the given string with a given character, such that the size is at least the given width.
	/// @tparam TString String type.
	/// @tparam TChar Character type. By default, it is the same character type aused by the string type.
	/// @param str String to pad.
	/// @param chr Character to pad with.
	/// @param width Minimum width the resulting string must have.
	/// @param just Direction to justify.
	/// @return Padded string.
	/// @note If `width <= str.size()`, then no modifications are applied.
	template<Type::OneOf<String, UTF8String, UTF32String> TString = String, Type::OneOf<char, UTF8Char, UTF32Char> TChar = typename TString::DataType>
	constexpr TString pad(TString const& str, TChar const chr, usize const width, Justify const& just = Justify::CFJ_LEFT) {
		if(width > str.size()) {
			switch (just) {
				case Justify::CFJ_LEFT:		return TString().resize(width - str.size(), TChar{chr}) + str;	break;
				case Justify::CFJ_RIGHT:	return str + TString().resize(width - str.size(), TChar{chr});	break;
				case Justify::CFJ_CENTER:
					usize lhs = usize((float(width) * 0.75) + 0.5);
					return pad(
						pad(
							str,
							chr,
							lhs,
							Justify::CFJ_LEFT
						),
						chr,
						width,
						Justify::CFJ_RIGHT
					);
				break;
			}
		}
		return str;
	}

	/// @brief Converts a floating point number to string, with formatting.
	/// @tparam T Floating point type.
	/// @tparam TString String type.
	/// @param num Number to convert.
	/// @param precision Amount of decimal places to have.
	/// @param minlead Minimum amount of whole digits to have.
	/// @return Resulting number string.
	template <Type::Real T, Type::OneOf<String, UTF8String, UTF32String> TString = String>
	constexpr TString prettify(T const& num, usize const precision, usize const minlead) {
		TString strnum = String::fromNumber<T>(num, precision);
		usize lead	= strnum.size() + minlead;
		ssize dot	= strnum.find('.');
		if (dot > -1) lead -= dot;
		return pad(strnum, '0', lead);
	}

	/// @brief Converts an integer to string, with formatting.
	/// @tparam T Integer type.
	/// @tparam TString String type.
	/// @param num Number to convert.
	/// @param precision Amount of decimal places to have.
	/// @param minlead Minimum amount of whole digits to have.
	/// @return Resulting number string.
	template <Type::Integer T, Type::OneOf<String, UTF8String, UTF32String> TString = String>
	constexpr TString prettify(T const& num, usize const precision, usize const minlead) {
		return prettify<floatmax>(num, precision, minlead);
	}
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_FORMAT_H
