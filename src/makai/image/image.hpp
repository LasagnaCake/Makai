#ifndef MAKAILIB_IMAGE_IMAGE_H
#define MAKAILIB_IMAGE_IMAGE_H

#include "../compat/ctl.hpp"
#include "get.hpp"

namespace Makai::Image::I2D {
	enum class Format {
		MI2F_INVALID = -1,
		MI2F_UNKNOWN,
		MI2F_TGA,
		MI2F_PNG,
		MI2F_JPG,
		MI2F_BMP,
	};

	enum class Channel: uint8 {
		MI2C_R		= 0b1000,
		MI2C_G		= 0b0100,
		MI2C_B		= 0b0010,
		MI2C_A		= 0b0001,
		MI2C_RGB	= MI2C_R | MI2C_G | MI2C_B,
		MI2C_RGBA	= MI2C_RGB | MI2C_A,
		MI2C_L		= MI2C_R,
		MI2C_LA		= MI2C_R | MI2C_A,
	};

	constexpr Channel operator|(Channel const a, Channel const b) {
		return Channel{enumcast(a) | enumcast(b)};
	}

	constexpr Channel operator&(Channel const a, Channel const b) {
		return Channel{enumcast(a) & enumcast(b)};
	}

	constexpr Channel operator~(Channel const ch) {
		return Channel{~enumcast(ch)};
	}

	constexpr bool operator!(Channel const ch) {
		return !enumcast(ch);
	}

	struct Attributes {
		usize	width		= -1;
		usize	height		= -1;
		Channel	channels	= Channel::MI2C_RGBA;
	};

	struct Image : Attributes {
		Bytes<>	data;
	};
}
