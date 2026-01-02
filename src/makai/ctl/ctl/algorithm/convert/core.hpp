#ifndef CTL_ALGORITHM_CONVERT_CORE_H
#define CTL_ALGORITHM_CONVERT_CORE_H

#include "../../ctypes.hpp"
#include "../../container/strings/strings.hpp"
#include "base.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Conversion facilities.
namespace Convert {
	constexpr usize strideof(Base const base) {
		switch (base) {
			case Base::CB_BASE2:	return 8; break;
			case Base::CB_BASE4:	return 4; break;
			case Base::CB_BASE8:	return 3; break;
			case Base::CB_BASE16:	return 2; break;
			default: break;
		}
		return 0;
	}

	constexpr char toBase32Char(byte const b) {
		constexpr auto const chs = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
		return chs[b % 32];
	}

	constexpr char toBase64Char(byte const b) {
		constexpr auto const chs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
		return chs[b % 64];
	}
}

CTL_NAMESPACE_END

#endif
