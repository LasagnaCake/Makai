#ifndef MAKAILIB_DATA_TOTP_H
#define MAKAILIB_DATA_TOTP_H

#include "../compat/ctl.hpp"

/// @brief Data manipulation facilities.
namespace Makai::Data {
	void totp(Span<byte const> const& secret, usize const timeIntervalInSeconds = 0);
	void totp(Span<char const> const& secret, usize const timeIntervalInSeconds = 0);
}

#endif // MAKAILIB_DATA_TOTP_H
