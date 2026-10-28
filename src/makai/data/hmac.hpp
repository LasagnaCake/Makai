#ifndef MAKAILIB_DATA_HMAC_H
#define MAKAILIB_DATA_HMAC_H

#include "hash.hpp"

/// @brief Data manipulation facilities.
namespace Makai::Data {
	String hmac(String const& message, String const& key, HashMode const mode);
}

#endif // MAKAILIB_DATA_HMAC_H
