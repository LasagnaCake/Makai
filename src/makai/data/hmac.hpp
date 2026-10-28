#ifndef MAKAILIB_DATA_HMAC_H
#define MAKAILIB_DATA_HMAC_H

#include "hash.hpp"

/// @brief Data manipulation facilities.
namespace Makai::Data {
	String hmac(String const& message, String const& key, HashMode const mode);

	struct HMAC {
		String key;
		HashMode mode;

		String get(String const& message) const;
	};
}

#endif // MAKAILIB_DATA_HMAC_H
