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
		};
	}

	/// @brief Converts binary data to a string.
	/// @tparam B Base to convert to.
	/// @param bin Binary data to convert.
	/// @return String.
	template <Base B>
	#ifdef __clang__
	[[clang::unavailable("Unimplemented!")]]
	#else
	[[gnu::unavailable("Unimplemented!")]]
	#endif
	constexpr String toBase(BinaryData<> const& bin) {
		return "";
	}
}

CTL_NAMESPACE_END

#endif
