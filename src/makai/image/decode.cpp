#include "decode.hpp"

using namespace Makai;

using namespace Makai::Image::I2D;

int readFromStream(pointer const inputStream, ref<bytes> const out, int const count) {
	auto& in = *(ref<IInputStream<Bytes<>>>)(inputStream);
	return in.tryReadInto(out, count);
}

void jumpInStream(pointer const inputStream, int const to) {
	auto& in = *(ref<IInputStream<Bytes<>>>)(inputStream);
	in.go(in.location() + to);
}

int isAtEnd(pointer const inputStream, int const to) {
	auto& in = *(ref<IInputStream<Bytes<>>>)(inputStream);
	return in.atEnd();
}

Nullable<Image> I2D::decodeStream(IInputStream<Bytes<>>& stream, Format const format) {
	if (format >= Format::MI2F_QOI) {
		// TODO: QOI decoder
		return null;
	} else {
		int imgWidth, imgHeight;
		int nrChannels;
		Bytes<> imgdat = File::getBinary(path);
		stbi_io_callbacks calls {readFromStream, jumpInStream, isAtEnd};
		owner<byte> data = stbi_load_from_callbacks(&calls, (pointer)&stream, &imgWidth, &imgHeight, &nrChannels, 4);
		imgdat.clear();
		if (data) {
			auto const result = Image{.width = imgWidth, .height = imgHeight, .data = {data, imgWidth * imgHeight * nrChannels}};
			stbi_image_free(data);
			return result;
		}
		return null;
	}
}

Nullable<Image> I2D::decode(ByteSpan<> const& data, Format const format) {
	InputMemoryStream stream(data);
	return decodeStream(stream, format);
}

Nullable<Image> I2D::decode(Bytes<> const& data, Format const format) {
	return decodeStream(ByteSpan<>(data.cbegin(), data.cend()), format);
}
