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
			/// @brief Converts binary data to a string.
			/// @param data Data to convert.
			/// @return String.
			constexpr static String convert(BinaryData<> const& data)
			requires (B < Base::CB_BASE32) {
				String result;
				for (auto const b: data) {
					switch (B) {
						case Base::CB_BASE2:	result += String::fromNumber<byte>(b, 2).substring(2);	break;
						case Base::CB_BASE4:	result += String::fromNumber<byte>(b, 4).substring(2);	break;
						case Base::CB_BASE8:	result += String::fromNumber<byte>(b, 8).substring(2);	break;
						case Base::CB_BASE16:	result += String::fromNumber<byte>(b, 16).substring(2);	break;
					}
				}
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
		};
	}

	/// @brief Converts binary data to a string.
	/// @tparam B Base to convert to.
	/// @param bin Binary data to convert.
	/// @return String.
	template <Base B>
	constexpr String toBase(BinaryData<> const& bin) {
		return "";
	}
}

CTL_NAMESPACE_END

#endif
