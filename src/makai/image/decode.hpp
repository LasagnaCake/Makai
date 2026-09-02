#ifndef MAKAILIB_IMAGE_DECODE_H
#define MAKAILIB_IMAGE_DECODE_H

#include "core.hpp"

namespace Makai::Image::I2D {
	Bytes<> decode(Bytes<> const& data, Format const format);
}

#endif
