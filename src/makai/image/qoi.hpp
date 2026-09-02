#ifndef MAKAILIB_IMAGE_QOI_H
#define MAKAILIB_IMAGE_QOI_H

#include "core.hpp"
#include "../graph/color/color.hpp"

namespace Makai::Image::I2D::QOI {
	using Color8 = Graph::Color::Color8;
	[[gnu::packed, gnu::aligned(1)]]
	struct Header {
		scstring<4>	magic = "qoif";
		uint32		width;
		uint32		height;
		uint8		channels	= 4;
		uint8		space		= 1;
	};
}

#endif
