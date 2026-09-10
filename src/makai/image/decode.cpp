#include "decode.hpp"
#include <stb_image.h>

using namespace Makai;

using namespace Makai::Image::I2D;

namespace I2D = Image::I2D;

static IInputStream<Bytes<>>& streamof(pointer const ptr) {
	return *(ref<IInputStream<Bytes<>>>)(ptr);
}

int readFromStream(pointer const inputStream, ref<char> const out, int const count) {
	auto& in = streamof(inputStream);
	return in.tryReadInto((ref<byte>)out, count);
}

void jumpInStream(pointer const inputStream, int const to) {
	auto& in = streamof(inputStream);
	in.go(in.position() + to);
}

int isAtEnd(pointer const inputStream) {
	auto& in = streamof(inputStream);
	return in.atEnd();
}

Nullable<I2D::Image> I2D::decodeStream(IInputStream<Bytes<>>& stream, Format const format) {
	if (format >= Format::MI2F_QOI) {
		// TODO: QOI decoder
		return null;
	} else {
		int imgWidth, imgHeight;
		int nrChannels;
		stbi_io_callbacks calls {readFromStream, jumpInStream, isAtEnd};
		owner<byte> data = stbi_load_from_callbacks(&calls, (pointer)&stream, &imgWidth, &imgHeight, &nrChannels, 4);
		if (data) {
			Image result;
			result.width = imgWidth;
			result.height = imgHeight;
			result.data = decltype(result.data)(data, Cast::as<usize>(imgWidth * imgHeight * nrChannels));
			stbi_image_free(data);
			return result;
		}
		return null;
	}
}

Nullable<I2D::Image> I2D::decode(ConstByteSpan<> const& data, Format const format) {
	InputMemoryStream stream(data);
	return decodeStream(stream, format);
}

Nullable<I2D::Image> I2D::decode(Bytes<> const& data, Format const format) {
	return decode(ConstByteSpan<>(data.data(), data.data() + data.size()), format);
}
