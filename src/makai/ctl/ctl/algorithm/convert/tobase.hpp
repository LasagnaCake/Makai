#ifndef CTL_ALGORITHM_CONVERT_TOBASE_H
#define CTL_ALGORITHM_CONVERT_TOBASE_H

#include "base.hpp"
#include "core.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Conversion facilities.
namespace Convert {
	/// @brief Implementation details.
	namespace Impl {
		/// @brief Binary-to-string conversion.
		/// @tparam B Base to convert to.
		template<Base B>
		struct BinaryToStringConverter {
			constexpr static String convert(byte b)
			requires (B < Base::CB_BASE32) {
				constexpr auto base = (1 << (enumcast(B) + 1));
				String s;
				do s += toBase32Char(b % base);
				while (b /= base);
				return String(strideof(B) - s.size(), '0') + s;
			}

			/// @brief Converts binary data to a string.
			/// @param data Data to convert.
			/// @return String.
			constexpr static String convert(BinaryData<> const& data)
			requires (B < Base::CB_BASE32) {
				if (data.empty()) return "";
				String result = "";
				for (auto const b: data)
					result += convert(b);
				return result;
			}

			/// @brief Converts binary data to a string.
			/// @param data Data to convert.
			/// @return String.
			constexpr static String convert(BinaryData<> const& data)
			requires (B == Base::CB_BASE32) {
				if (data.empty()) return "";
				String result = "";
				usize i = 0;
				for (i = 0; i < data.size(); i += 5) {
					result += b32part({data.data() + i, 5});
				}
				if (data.size() % 5)
					result += b32part({data.data() + i - 5, data.size() % 5});
				return result;
			}
			
			/// @brief Converts binary data to a string.
			/// @param data Data to convert.
			/// @return String.
			constexpr static String convert(BinaryData<> const& data)
			requires (B == Base::CB_BASE64) {
				if (data.empty()) return "";
				String result = "";
				usize i = 0;
				for (i = 0; i < data.size(); i += 3) {
					result += b64part({data.data() + i, 3});
				}
				if (data.size() % 3)
					result += b64part({data.data() + i - 3, data.size() % 3});
				return result;
			}

		private:
			constexpr static String b32part(ConstByteSpan<> const& data) {
				constexpr byte const COFF = 31;
				String s;
				s.pushBack(toBase32Char((data[0] >> 3) & COFF));
				if (data.size() < 2) return s + String(8 - s.size(), '=');
				s.pushBack(toBase32Char(((data[0] << 2) | (data[1] >> 6)) & COFF));
				s.pushBack(toBase32Char((data[1] >> 1) & COFF));
				if (data.size() < 3) return s + String(8 - s.size(), '=');
				s.pushBack(toBase32Char(((data[1] << 4) | (data[2] >> 4)) & COFF));
				if (data.size() < 4) return s + String(8 - s.size(), '=');
				s.pushBack(toBase32Char(((data[2] << 1) | (data[3] >> 7)) & COFF));
				s.pushBack(toBase32Char((data[3] >> 2) & COFF));
				if (data.size() < 5) return s + String(8 - s.size(), '=');
				s.pushBack(toBase32Char(((data[3] << 3) | (data[4] >> 5)) & COFF));
				s.pushBack(toBase32Char(data[4] & COFF));
				return s;
			}

			constexpr static String b64part(ConstByteSpan<> const& data) {
				constexpr byte const COFF = 63;
				String s;
				// TODO: This
				s.pushBack(toBase64Char((data[0] >> 2) & COFF));
				if (data.size() < 2) return s + String(4 - s.size(), '=');
				s.pushBack(toBase64Char(((data[0] << 4) | (data[1] >> 4)) & COFF));
				if (data.size() < 3) return s + String(4 - s.size(), '=');
				s.pushBack(toBase64Char(((data[1] << 2) | (data[2] >> 6)) & COFF));
				s.pushBack(toBase64Char(data[2] & COFF));
				return s;
			}
		};
	}

	/// @brief Converts binary data to a string.
	/// @tparam B Base to convert to.
	/// @param bin Binary data to convert.
	/// @return String.
	template <Base B>
	constexpr String toBase(BinaryData<> const& bin) {
		return Impl::BinaryToStringConverter<B>::convert(bin);
	}
}

CTL_NAMESPACE_END

#endif
