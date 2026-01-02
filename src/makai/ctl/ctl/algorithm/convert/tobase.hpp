#ifndef CTL_ALGORITHM_CONVERT_TOBASE_H
#define CTL_ALGORITHM_CONVERT_TOBASE_H

#include "../../container/strings/strings.hpp"
#include "base.hpp"

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
				auto const c = (b % base) + '0';
				do s += Cast::as<char>(c < 0x3A ? c : c + (0x40 - 0x3A));
				while (b /= base);
				return String(stride(B) - s.size(), '0') + s;
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
			#ifdef __clang__
			[[clang::unavailable("Unimplemented!")]]
			#else
			[[gnu::unavailable("Unimplemented!")]]
			#endif
			constexpr static String convert(BinaryData<> const& data)
			requires (B == Base::CB_BASE32) {
				return "";
			}
			
			/// @brief Converts binary data to a string.
			/// @param data Data to convert.
			/// @return String.
			#ifdef __clang__
			[[clang::unavailable("Unimplemented!")]]
			#else
			[[gnu::unavailable("Unimplemented!")]]
			#endif
			constexpr static String convert(BinaryData<> const& data)
			requires (B == Base::CB_BASE64) {
				return "";
			}

		private:
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
