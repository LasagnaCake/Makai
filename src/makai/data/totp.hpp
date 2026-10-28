#ifndef MAKAILIB_DATA_TOTP_H
#define MAKAILIB_DATA_TOTP_H

#include "../compat/ctl.hpp"

/// @brief Data manipulation facilities.
namespace Makai::Data {
	String totp(String const& secret, usize const interval = 30, usize const digits = 6, HashMode const mode = HashMode::HM_SHA1);
}

#endif // MAKAILIB_DATA_TOTP_H
