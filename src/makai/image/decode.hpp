#ifndef MAKAILIB_IMAGE_DECODE_H
#define MAKAILIB_IMAGE_DECODE_H

#include "core.hpp"

namespace Makai::Image::I2D {
	Nullable<Image> decodeStream(IInputStream<Bytes<>>& data, Format const format);
	Nullable<Image> decode(ByteSpan<> const& data, Format const format);
	Nullable<Image> decode(Bytes<> const& data, Format const format);
}

#endif
