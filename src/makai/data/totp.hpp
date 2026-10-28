#ifndef MAKAILIB_DATA_TOTP_H
#define MAKAILIB_DATA_TOTP_H

#include "../compat/ctl.hpp"

#include "hash.hpp"

/// @brief Data manipulation facilities.
namespace Makai::Data {
	String totp(String const& secret, i64 const time, u64 const interval = 30, usize const digits = 6, HashMode const mode = HashMode::HM_SHA1);

	struct TOTP {
		String		secret;
		u64			interval	= 30;
		usize		digits		= 6;
		HashMode	mode		= HashMode::HM_SHA1;

		String get() const;

		operator String() const {return get();}
	};
}

#endif // MAKAILIB_DATA_TOTP_H
