#ifndef CTL_ALGORITHM_CONVERT_FROMBASE_H
#define CTL_ALGORITHM_CONVERT_FROMBASE_H

#include "../../container/strings/strings.hpp"
#include "base.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Conversion facilities.
namespace Convert {
	/// @brief Implementation details.
	namespace Impl {
		/// @brief String-to-binary conversion.
		/// @tparam B Base to convert from.
		template<Base B>
		struct StringToBinaryConverter {
			/// @brief Converts a string to binary data.
			/// @param str String to convert.
			/// @return Binary data.
			constexpr static BinaryData<> convert(String const& str)
			requires (B < Base::CB_BASE32) {
				usize start = 0;
				BinaryData<> result;
				while (start < str.size()) {
					result.pushBack(String::toNumber<byte>(str.substring(start, start + 2), stride(B)));
					start += stride(B);
				}
				return result;
			}

			/// @brief Converts a string to binary data.
			/// @param str String to convert.
			/// @return Binary data.
			constexpr static BinaryData<> convert(String const& str)
			requires (B == Base::CB_BASE32) {
				if (str.size() % 8 != 0) return convert(str + String("=") * (str.size() % 8));
				usize current = 0;
				BinaryData<> result;
				while (current < str.size()) {
					auto const section = str.substring(current, 8).upper().transform(toBase32Value);
					result.expand(5);
					result.pushBack(0);
					result.back() |= ((Cast::as<uint8>(section[0]) & 0b11111) << 3);
					if (section[1] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[1]) & 0b11100) >> 2);
					result.pushBack(0);
					result.back() |= ((Cast::as<uint8>(section[1]) & 0b00011) << 6);
					if (section[2] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[2]) & 0b11111) << 1);
					if (section[3] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[3]) & 0b10000) >> 4);
					result.pushBack(0);
					result.back() |= ((Cast::as<uint8>(section[3]) & 0b01111) << 4);
					if (section[4] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[4]) & 0b11110) >> 1);
					result.pushBack(0);
					result.back() |= ((Cast::as<uint8>(section[4]) & 0b00001) << 7);
					if (section[5] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[5]) & 0b11111) << 2);
					if (section[6] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[6]) & 0b11000) >> 3);
					result.pushBack(0);
					result.back() |= ((Cast::as<uint8>(section[6]) & 0b00111) << 5);
					if (section[7] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[7]) & 0b11111));
					current += 8;
				}
				return result;
			}

			/// @brief Converts a string to binary data.
			/// @param str String to convert.
			/// @return Binary data.
			constexpr static BinaryData<> convert(String const& str)
			requires (B == Base::CB_BASE64) {
				if (str.size() % 4 != 0) return convert(str + String("=") * (str.size() % 4));
				usize current = 0;
				BinaryData<> result;
				while (current < str.size()) {
					auto const section = str.substring(current, 4).transform(toBase64Value);
					result.expand(3);
					result.pushBack(0);
					result.back() |= ((Cast::as<uint8>(section[0]) & 0b111111) << 2);
					if (section[1] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[1]) & 0b110000) >> 4);
					result.pushBack(0);
					result.back() |= ((Cast::as<uint8>(section[1]) & 0b001111) << 4);
					if (section[2] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[2]) & 0b111100) >> 2);
					result.pushBack(0);
					result.back() |= ((Cast::as<uint8>(section[2]) & 0b000011) << 6);
					if (section[3] == 127) break;
					result.back() |= ((Cast::as<uint8>(section[3]) & 0b111111));
					current += 4;
				}
				return result;
			}

		private:
			constexpr static char toBase32Value(char c) {
				if (c == '=') return 127;
				c = toUpperChar(c);
				if (c < 'A') return c - '0';
				return c - 'A' + 10;
			}

			constexpr static char toBase64Value(char const c) {
				if (c == '=') return 127;
				if (c == '+' || c == '-') return 62;
				if (c == '/' || c == '_') return 63;
				if (c < 'A') return c - '0' + 52;
				if (c < 'a') return c - 'A';
				return c - 'a' + 26;
			}

			consteval static usize stride(Base const base) {
				switch (base) {
					case Base::CB_BASE2:	return 8; break;
					case Base::CB_BASE4:	return 4; break;
					case Base::CB_BASE8:	return 3; break;
					case Base::CB_BASE16:	return 2; break;
					default: break;
				}
				return 0;
			}
		};
	}

	/// @brief Converts a string to binary data.
	/// @tparam B Base to convert from.
	/// @param str String to convert.
	/// @return Binary data.
	template <Base B>
	constexpr BinaryData<> fromBase(String const& str) {
		return Impl::StringToBinaryConverter<B>::convert(str);
	}
}

CTL_NAMESPACE_END

#endif
