#ifndef MAKAILIB_IMAGE_CORE_H
#define MAKAILIB_IMAGE_CORE_H

#include "../compat/ctl.hpp"
#include "get.hpp"

namespace Makai::Image::I2D {
	enum class Format: uint64 {
		MI2F_INVALID = Limit::MAX<uint64>,
		MI2F_UNKNOWN,
		MI2F_TGA,
		MI2F_PNG,
		MI2F_JPG,
		MI2F_BMP,
		MI2F_QOI,
	};

	[[gnu::aligned(1)]]
	struct [[CTL_FLAG_STRUCT(uint8)]] Channel {
		uint8 r: 1;
		uint8 g: 1;
		uint8 b: 1;
		uint8 a: 1;

		constexpr static RGBA()	{return Channel{1, 1, 1, 1};	}
		constexpr static RGB()	{return Channel{1, 1, 1, 0};	}
		constexpr static RA()	{return Channel{1, 0, 0, 1};	}
		constexpr static LA()	{return RA();					}
	};

	struct Attributes {
		uint64	width		= 0;
		uint64	height		= 0;
		Channel	channels	= Channel::RGBA();
	};

	struct Image : Attributes {
		Bytes<>	data;
	};
}
