#include "glapiloader.cc"

#include <stb_image.h>
#include <stb_image_write.h>

#include "texture.hpp"
#include "../../data/encdec.hpp"
#include "../../file/get.hpp"

using namespace Makai; using namespace Makai::Graph;

using WrapMode			= Texture2D::WrapMode;
using FilterMode		= Image2D::FilterMode;
using ImageFileType		= Image2D::ImageFileType;
using ImageFormat		= Image2D::ImageFormat;
using ComponentType		= Image2D::ComponentType;
using ComponentLayout	= Image2D::ComponentLayout;
using ImageTarget		= Image::ImageTarget;

constexpr uint32 convert(WrapMode const& mode) {
	switch (mode) {
		default:
		case WrapMode::WM_REPEAT:			return GL_REPEAT;
		case WrapMode::WM_CLAMP:			return GL_CLAMP_TO_EDGE;
		case WrapMode::WM_MIRROR_REPEAT:	return GL_MIRRORED_REPEAT;
		case WrapMode::WM_MIRROR_CLAMP:		return GL_MIRROR_CLAMP_TO_EDGE;
	}
}

constexpr uint32 convert(FilterMode const& type) {
	switch (type) {
		default:
		case FilterMode::FM_NEAREST:	return GL_NEAREST;
		case FilterMode::FM_SMOOTH:		return GL_LINEAR;
		case FilterMode::FM_NMN:		return GL_NEAREST_MIPMAP_NEAREST;
		case FilterMode::FM_NMS:		return GL_NEAREST_MIPMAP_LINEAR;
		case FilterMode::FM_SMN:		return GL_LINEAR_MIPMAP_NEAREST;
		case FilterMode::FM_SMS:		return GL_LINEAR_MIPMAP_LINEAR;
	}
}

uint32 createCopyBuffer() {
	DEBUGLN("Creating copy buffer...");
	uint id = 0;
	glGenFramebuffers(1, &id);
	return id;
}

void copyTexture(
	Image2D* const		src,
	Image2D* const		dst,
	uint32 const		srcStartX,
	uint32 const		srcStartY,
	uint32 const		srcEndX,
	uint32 const		srcEndY,
	uint32 const		dstStartX,
	uint32 const		dstStartY,
	uint32 const		dstEndX,
	uint32 const		dstEndY,
	FilterMode const&	filter = FilterMode::FM_NEAREST
) {
	if (!src || !dst) return;
	static uint32 const fb = createCopyBuffer();
	DEBUGLN("Binding copy buffer...");
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	DEBUGLN("Binding source...");
	glFramebufferTexture2D(
		GL_READ_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		src->getID(),
		0
	);
	DEBUGLN("Binding destination...");
	glFramebufferTexture2D(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D,
		dst->getID(),
		0
	);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	DEBUGLN((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ? "OK" : "ERR"));
	DEBUGLN("Copying textures...");
	glBlitFramebuffer(
		srcStartX, srcStartY, srcEndX, srcEndY,
		dstStartX, dstStartY, dstEndX, dstEndY,
		GL_COLOR_BUFFER_BIT, convert(filter)
	);
	DEBUGLN("Finalizing...");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Texture2D Texture2D::fromJSON(JSON::Value img, String const& sourcepath) {
	Texture2D texture;
	if (img["data"].isObject() && img["data"]["path"].isString() && !img["data"]["path"].get<String>().empty()) {
		texture.create(OS::FS::concatenate(sourcepath, img["path"].get<String>()));
		texture.setFilterMode(
			img["minFilter"].get<FilterMode>(FilterMode::FM_NMN),
			img["magFilter"].get<FilterMode>(FilterMode::FM_NEAREST)
		);
	} else if (img["data"].isString() && !img["data"].get<String>().empty()) {
		List<ubyte> data = Makai::Data::decode(img["data"].get<String>(), Makai::Data::fromString((String)img["encoding"]));
		int w, h, nc;
		uint8* imgdat = stbi_load_from_memory(
			data.data(),
			data.size(),
			&w,
			&h,
			&nc,
			4
		);
		if (imgdat) {
			texture.create(
				w,
				h,
				ComponentType::CT_UBYTE,
				ImageFormat::IF_RGBA,
				img["minFilter"].get<FilterMode>(FilterMode::FM_NMN),
				img["magFilter"].get<FilterMode>(FilterMode::FM_NEAREST),
				imgdat
			);
			stbi_image_free(imgdat);
		} else throw Error::FailedAction(
			"Failed at getting image effect!",
			"Could not decode embedded image data!",
			"Please check to see if values are correct!",
			CTL_CPP_PRETTY_SOURCE
		);
	}
	return texture;
}

Texture2D::Texture2D(
	uint32 const			width,
	uint32 const			height,
	ComponentType const&	type,
	ImageFormat const&		format,
	FilterMode const&		magFilter,
	FilterMode const&		minFilter,
	uint8* const			data,
	ComponentLayout const&	layout
): Texture2D::Texture2D() {
	create(
		width,
		height,
		type,
		format,
		minFilter,
		magFilter,
		data,
		layout
	);
}

Texture2D::Texture2D(
	String const& path,
	FilterMode const& magFilter,
	FilterMode const& minFilter
): Texture2D::Texture2D() {
	create(path, minFilter, magFilter);
}

Texture2D::Texture2D(
	Image2D::ImageData const& image
): Texture2D::Texture2D() {
	create(image);
}

Texture2D::Texture2D(
	Texture2D const& other
) {
	image = other.image;
}

Texture2D::Texture2D(
	Texture2D const& other,
	uint32 const startX,
	uint32 const startY,
	uint32 const endX,
	uint32 const endY,
	bool const filter
): Texture2D::Texture2D() {
	create(other, startX, startY, endX, endY, filter);
}

Texture2D& Texture2D::create(
	uint32 const			width,
	uint32 const			height,
	ComponentType const&	type,
	ImageFormat const&		format,
	FilterMode const&		magFilter,
	FilterMode const&		minFilter,
	uint8* const			data,
	ComponentLayout const&	layout
) {
	if (exists()) return *this;
	if (!image)
		image = Image2D::newImage(
			width,
			height,
			type,
			format,
			minFilter,
			magFilter,
			data,
			layout
		);
	else if (image && !image->exists())
		Image2D::newImage(
			(Image2D*)image,
			width,
			height,
			type,
			format,
			minFilter,
			magFilter,
			data,
			layout
		);
	setWrapMode();
	return *this;
}

Texture2D& Texture2D::create(
	String const& path,
	FilterMode const& magFilter,
	FilterMode const& minFilter
) {
	if (exists()) return *this;
	int imgWidth, imgHeight;
	int nrChannels;
	BinaryData<> imgdat = File::getBinary(path);
	uint8* data = stbi_load_from_memory(imgdat.data(), imgdat.size(), &imgWidth, &imgHeight, &nrChannels, 4);
	imgdat.clear();
	if (data) {
		create(
			imgWidth,
			imgHeight,
			ComponentType::CT_UBYTE,
			ImageFormat::IF_RGBA,
			minFilter,
			magFilter,
			data
		);
		stbi_image_free(data);
	} else {
		throw Error::FailedAction(
			"Could not load image file '" + path + "'!\n\n",
			stbi_failure_reason(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	return *this;
}

Texture2D& Texture2D::create(
	Image2D::ImageData const& image
) {
	if (exists()) return *this;
	DEBUGLN("copying textures...");
	if (image.data.empty()) return *this;
	DEBUGLN("Texture data exists!");
	create(
		image.width,
		image.height,
		image.type,
		image.format,
		image.minFilter,
		image.magFilter,
		(unsigned char*)image.data.data(),
		image.layout
	);
	return *this;
}

Texture2D& Texture2D::create(
	Texture2D const& other
) {
	if (exists()) return *this;
	image = other.image;
	return *this;
}

Texture2D& Texture2D::create(
	Texture2D const& other,
	uint32 const startX,
	uint32 const startY,
	uint32 const endX,
	uint32 const endY,
	bool const filter
) {
	if (exists()) return *this;
	uint32 w, h;
	w = Math::max(startX, endX) - Math::min(startX, endX);
	h = Math::max(startY, endY) - Math::min(startY, endY);
	create(
		w, h,
		other.image->attributes.type,
		other.image->attributes.format,
		other.image->attributes.minFilter,
		other.image->attributes.magFilter,
		NULL,
		other.image->attributes.layout
	);
	copyFrom(other, startX, startY, endX, endY, filter);
	return *this;
}

Texture2D& Texture2D::destroy() {
	if (!exists()) return *this;
	image.unbind();
	return *this;
}

Texture2D& Texture2D::clear() {
	if (!exists()) return *this;
	image->destroy();
	return *this;
}

Texture2D& Texture2D::make(
	uint32 const			width,
	uint32 const			height,
	ComponentType const&	type,
	ImageFormat const&		format,
	FilterMode const&		magFilter,
	FilterMode const&		minFilter,
	uint8* const			data,
	ComponentLayout const&	layout
) {
	destroy();
	create(width, height, type, format, magFilter, minFilter, data, layout);
	return *this;
}

Texture2D& Texture2D::make(
	String const& path,
	FilterMode const& magFilter,
	FilterMode const& minFilter
) {
	destroy();
	create(path, minFilter, magFilter);
	return *this;
}

Texture2D& Texture2D::make(
	Image2D::ImageData const& image
) {
	destroy();
	create(image);
	return *this;
}

Texture2D& Texture2D::make(
	Texture2D const& other
) {
	destroy();
	create(other);
	return *this;
}

Texture2D& Texture2D::make(
	Texture2D const& other,
	uint32 const startX,
	uint32 const startY,
	uint32 const endX,
	uint32 const endY,
	bool const filter
) {
	destroy();
	create(other, startX, startY, endX, endY, filter);
	return *this;
}

Texture2D& Texture2D::makeUnique(bool const filter) {
	if (!exists()) return *this;
	Image2D* newImg = Image2D::newImage(
		image->attributes.width,
		image->attributes.height,
		image->attributes.type,
		image->attributes.format,
		image->attributes.minFilter,
		image->attributes.magFilter,
		NULL,
		image->attributes.layout
	);
	copyTexture(
		(Image2D*)image, (Image2D*)newImg,
		0, 0, image->attributes.width, image->attributes.height,
		0, 0, image->attributes.width, image->attributes.height,
		filter ? FilterMode::FM_SMOOTH : FilterMode::FM_NEAREST
	);
	image = newImg;
	return *this;
}

Texture2D& Texture2D::operator=(Texture2D const& other)	{make(other); return *this;								}
Texture2D& Texture2D::operator=(Texture2D&& other)		{make(other); return *this;								}

bool Texture2D::operator==(Texture2D const& other) const		{return *image == *other.image;		}
ValueOrder Texture2D::operator<=>(Texture2D const& other) const	{return *image <=> *other.image;	}

Texture2D& Texture2D::copyFrom(
	Texture2D const& other,
	uint32 const startX,
	uint32 const startY,
	uint32 const endX,
	uint32 const endY,
	bool const filter
) {
	if (!exists()) return *this;
	// Copy data
	copyTexture(
		(Image2D*)other.image, (Image2D*)image,
		startX, startY, endX, endY,
		0, 0, image->attributes.width, image->attributes.height,
		filter ? FilterMode::FM_SMOOTH : FilterMode::FM_NEAREST
	);
	// Regenerate mipmaps
	glBindTexture(GL_TEXTURE_2D, image->getID());
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return *this;
}

Texture2D& Texture2D::copyFrom(
	Texture2D const& other,
	bool const filter
) {
	if (!exists()) return *this;
	// Copy data
	copyTexture(
		(Image2D*)other.image, (Image2D*)image,
		0, 0, other.image->attributes.width, other.image->attributes.height,
		0, 0, image->attributes.width, image->attributes.height,
		filter ? FilterMode::FM_SMOOTH : FilterMode::FM_NEAREST
	);
	// Regenerate mipmaps
	glBindTexture(GL_TEXTURE_2D, image->getID());
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return *this;
}

Texture2D& Texture2D::setWrapMode(
	Texture2D::WrapMode const& horizontal,
	Texture2D::WrapMode const& vertical
) {
	if (!exists()) return *this;
	glBindTexture(GL_TEXTURE_2D, image->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, convert(horizontal));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, convert(vertical));
	glBindTexture(GL_TEXTURE_2D, 0);
	return *this;
}

Texture2D& Texture2D::setWrapMode(Texture2D::WrapMode const& mode) {
	setWrapMode(mode, mode);
	return *this;
}

Texture2D& Texture2D::setFilterMode(
	FilterMode const& magFilter,
	FilterMode const& minFilter
) {
	if (!exists()) return *this;
	glBindTexture(GL_TEXTURE_2D, image->getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, convert(minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, convert(magFilter));
	glBindTexture(GL_TEXTURE_2D, 0);
	image->attributes.minFilter = minFilter;
	image->attributes.magFilter = magFilter;
	return *this;
}

FilterMode Texture2D::minFilter() const {
	return image->attributes.minFilter;
}

FilterMode Texture2D::magFilter() const {
	return image->attributes.magFilter;
}

Image2D::Attributes Texture2D::attributes() const {
	return image->attributes;
}

Texture2D& Texture2D::enable(uint8 const slot) {
	image->use(slot);
	return *this;
}

Texture2D const& Texture2D::enable(uint8 const slot) const {
	image->use(slot);
	return *this;
}

Texture2D& Texture2D::operator()(uint8 const slot) {
	return enable(slot);
}

Texture2D const& Texture2D::operator()(uint8 const slot) const {
	return enable(slot);
}

uint32 Texture2D::getID() const {
	if (!exists()) return 0;
	return image->getID();
}

Image2D::ImageData Texture2D::getData() const {
	return image->getData();
}

Texture2D const& Texture2D::saveToFile(
	String const& path,
	uint8 const quality,
	ImageFileType const& type
) const {
	if (!exists()) return *this;
	image->saveToFile(path, quality, type);
	return *this;
}

Texture2D& Texture2D::saveToFile(
	String const& path,
	uint8 const quality,
	ImageFileType const& type
) {
	if (!exists()) return *this;
	image->saveToFile(path, quality, type);
	return *this;
}

Texture2D const& Texture2D::saveToFile(
	String const& path,
	ImageFileType const& type
) const {
	return saveToFile(path, 50, type);
}

Texture2D& Texture2D::saveToFile(
	String const& path,
	ImageFileType const& type
) {
	return saveToFile(path, 50, type);
}

bool Texture2D::exists() const {
	return image.exists() && image->exists();
}

Texture2D::operator bool() const {
	return exists();
}
