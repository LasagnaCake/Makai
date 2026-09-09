#ifndef MAKAILIB_IMAGE_QOI_H
#define MAKAILIB_IMAGE_QOI_H

#include "core.hpp"

namespace Makai::Image::I2D::QOI {
	using Color8 = Color::RGBAi8;
	struct [[CTL_PACKED_STRUCT]] Header {
		scstring<4> const	magic = {'q', 'o', 'i', 'f'};
		uint32				width;
		uint32				height;
		uint8				channels	= 4;
		uint8				space		= 1;
	};
}

#endif
