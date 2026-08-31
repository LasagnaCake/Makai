#ifndef MAKAILIB_IMAGE_CORE_H
#define MAKAILIB_IMAGE_CORE_H

#include "../compat/ctl.hpp"
#include "../graph/color/color.hpp"
#include "get.hpp"

namespace Makai::Image::I2D::QOI {
	using Color8 = Graph::Color::Color8;
	[[gnu::packed, gnu::aligned(1)]]
	struct Header {
		As<char const[4]> magic = "qoif";
		uint32	width;
		uint32	height;
		uint8	channels	= 4;
		uint8	space		= 1;
	};
}
