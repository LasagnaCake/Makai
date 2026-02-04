#include "glapiloader.cc"

#include <stb_image.h>
#include <stb_image_write.h>

#include "image.hpp"

using namespace Makai; using namespace Makai::Graph;

using ImageTarget		= Image::ImageTarget;
using ImageFileType		= Image2D::ImageFileType;
using ComponentType		= Image2D::ComponentType;
using ComponentLayout	= Image2D::ComponentLayout;
using ImageFormat		= Image2D::ImageFormat;
using FilterMode		= Image2D::FilterMode;

using ImageData			= Image2D::ImageData;

constexpr uint convert(Image::ImageTarget const& target) {
	switch (target) {
		case Image::ImageTarget::IT_TEXTURE_2D: return GL_TEXTURE_2D;
	}
	return GL_FALSE;
}

constexpr ImageFileType fromFileExtension(String const& path) {
	String ext;
	auto sp = path.splitAtLast('.').back().lower();
	if (ext == "png")					return ImageFileType::IFT_PNG;
	if (ext == "jpg" || ext == "jpeg")	return ImageFileType::IFT_JPG;
	if (ext == "tga")					return ImageFileType::IFT_TGA;
	if (ext == "bmp")					return ImageFileType::IFT_BMP;
	return ImageFileType::IFT_INVALID;
}

constexpr uint convert(ComponentType const& type) {
	switch (type) {
		default:
		case ComponentType::CT_UBYTE:		return GL_UNSIGNED_BYTE;
		case ComponentType::CT_BYTE:		return GL_BYTE;
		case ComponentType::CT_USHORT:		return GL_UNSIGNED_SHORT;
		case ComponentType::CT_SHORT:		return GL_SHORT;
		case ComponentType::CT_HALF_FLOAT:	return GL_HALF_FLOAT;
		case ComponentType::CT_UINT:		return GL_UNSIGNED_INT;
		case ComponentType::CT_INT:			return GL_INT;
		case ComponentType::CT_FLOAT:		return GL_FLOAT;
		case ComponentType::CT_UINT_24_8:	return GL_UNSIGNED_INT_24_8;
	}
}

constexpr uint convert(ImageFormat const& type) {
	switch (type) {
		case ImageFormat::IF_D:		return GL_DEPTH_COMPONENT;
		case ImageFormat::IF_R:		return GL_RED;
		case ImageFormat::IF_DS:	return GL_DEPTH_STENCIL;
		case ImageFormat::IF_RG:	return GL_RG;
		case ImageFormat::IF_RGB:	return GL_RGB;
		default:
		case ImageFormat::IF_RGBA:	return GL_RGBA;
	}
}

constexpr uint convert(ComponentLayout const& type) {
	switch (type) {
		default:
		case ComponentLayout::CL_AUTO:		return 0;
		case ComponentLayout::CL_D:			return GL_DEPTH_COMPONENT;
		case ComponentLayout::CL_R:			return GL_RED;
		case ComponentLayout::CL_DS:		return GL_DEPTH_STENCIL;
		case ComponentLayout::CL_RG:		return GL_RG;
		case ComponentLayout::CL_RGB:		return GL_RGB;
		case ComponentLayout::CL_RGBA:		return GL_RGBA;
		case ComponentLayout::CL_RGBA_16F:	return GL_RGBA16F;
		case ComponentLayout::CL_D24_S8:	return GL_DEPTH24_STENCIL8;
	}
}

constexpr uint convert(FilterMode const& type) {
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

Image& Image::bind(ImageTarget const& target) {
	glBindTexture(convert(target), id);
	return *this;
}

Image const& Image::bind(ImageTarget const& target) const {
	glBindTexture(convert(target), id);
	return *this;
}

Image& Image::destroy() {
	if (!exists()) return *this;
	glDeleteTextures(1, &id);
	id = 0;
	return *this;
}

Image& Image::create() {
	if (exists()) return *this;
	glGenTextures(1, &id);
	return *this;
}

Image& Image::make() {
	return destroy().create();
}

void Image::unbind(ImageTarget const& target) {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("Image::unbind");
	#endif // MAKAILIB_DEBUG
	glBindTexture(convert(target), 0);
}

Image& Image::use(uint8 const slot, ImageTarget const& target) {
	set(id, slot, target);
	return *this;
}

Image const& Image::use(uint8 const slot, ImageTarget const& target) const {
	set(id, slot, target);
	return *this;
}

void Image::set(uint const image, uint8 const slot, ImageTarget const& target) {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("Image::set");
	#endif // MAKAILIB_DEBUG
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(convert(target), image);
}

Image& Image::operator()(uint8 const slot, ImageTarget const& target) {
	return use(slot);
}

Image const& Image::operator()(uint8 const slot, ImageTarget const& target) const {
	return use(slot);
}

bool Image::operator==(Image const& other) const {
	return id == other.id;
}

CTL::ValueOrder Image::operator<=>(Image const& other) const {
	return id <=> other.id;
}

Image::operator uint() const {
	return id;
}

uint Image::getID() const {
	return id;
}

bool Image::exists() const {
	return id != 0;
}

Image::operator bool() const {
	return exists();
}

Image2D& Image2D::create(
	uint const				width,
	uint const				height,
	ComponentType const&	type,
	ImageFormat const&		format,
	FilterMode const&		magFilter,
	FilterMode const&		minFilter,
	uint8* const			data,
	ComponentLayout const&	layout
) {
	if (exists()) return *this;
	newImage(this, width, height, type, format, minFilter, magFilter, data, layout);
	return *this;
}

Image2D& Image2D::make(
	uint const				width,
	uint const				height,
	ComponentType const&	type,
	ImageFormat const&		format,
	FilterMode const&		magFilter,
	FilterMode const&		minFilter,
	uint8* const			data,
	ComponentLayout const&	layout
) {
	destroy();
	return create(width, height, type, format, magFilter, minFilter, data, layout);
}

Image2D::Attributes Image2D::getAttributes() const {
	return attributes;
}

ImageData Image2D::getData() const {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("Image2D::getData");
	#endif // MAKAILIB_DEBUG
	DEBUGLN("Getting image data...");
	if (!exists()) return ImageData{
		0, 0,
		ComponentType::CT_BYTE,
		ImageFormat::IF_RGBA,
		ComponentLayout::CL_AUTO,
		FilterMode::FM_NEAREST,
		FilterMode::FM_NEAREST
	};
	usize size = 0;
	switch (attributes.type) {
		default:
		case ComponentType::CT_UBYTE:
		case ComponentType::CT_BYTE:		size = 1;	break;
		case ComponentType::CT_USHORT:
		case ComponentType::CT_SHORT:
		case ComponentType::CT_HALF_FLOAT:	size = 2;	break;
		case ComponentType::CT_UINT:
		case ComponentType::CT_INT:
		case ComponentType::CT_FLOAT:		size = 4;	break;
	}
	DEBUG("Image Size: ", size);
	switch (attributes.format) {
		case ImageFormat::IF_D:
		case ImageFormat::IF_R:					break;
		case ImageFormat::IF_DS:
		case ImageFormat::IF_RG:	size *= 2;	break;
		case ImageFormat::IF_RGB:	size *= 3;	break;
		default:
		case ImageFormat::IF_RGBA:	size *= 4;	break;
	}
	DEBUGLN("Image Pixel Width: ", size);
	DEBUGLN("Width: ", attributes.width);
	DEBUGLN("Height: ", attributes.height);
	DEBUGLN("Image Size: ", ((usize)attributes.width) * ((usize)attributes.height) * size);
	ImageData imgdat = (ImageData)attributes;
	DEBUGLN("Reserving buffer...");
	imgdat.data.resize(((usize)attributes.width) * ((usize)attributes.height) * size, 0);
	DEBUGLN("Reserved: ", imgdat.data.size());
	if (imgdat.data.empty()) throw Error::FailedAction("Somehow, the image data is empty.", CTL_CPP_PRETTY_SOURCE);
	DEBUGLN("Extracting pixels...");
	glBindTexture(GL_TEXTURE_2D, getID());
	glGetTexImage(GL_TEXTURE_2D, 0, convert(attributes.format), convert(attributes.type), imgdat.data.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	DEBUGLN("Done!");
	DEBUGLN("Reserved: ", imgdat.data.size());
	return imgdat;
}

Image2D const& Image2D::saveToFile(String const& path, uint8 const quality, ImageFileType const& type) const {
	saveImageToFile(path, quality, type);
	return *this;
}

Image2D& Image2D::saveToFile(String const& path, uint8 const quality, ImageFileType const& type) {
	saveImageToFile(path, quality, type);
	return *this;
}

Image2D& Image2D::saveToFile(String const& path, ImageFileType const& type) {
	return saveToFile(path, 50, type);
}

Image2D const& Image2D::saveToFile(String const& path, ImageFileType const& type) const	{
	return saveToFile(path, 50, type);
}

Image2D* Image2D::newImage(
	uint const				width,
	uint const				height,
	ComponentType const&	type,
	ImageFormat const&		format,
	FilterMode const&		magFilter,
	FilterMode const&		minFilter,
	uint8* const			data,
	ComponentLayout const&	layout,
	ImageTarget const&		target
) {
	Image2D* image = new Image2D();
	return Image2D::newImage(image, width, height, type, format, minFilter, magFilter, data, layout, target);
}

Image2D* Image2D::newImage(
	Image2D* const			image,
	uint const				width,
	uint const				height,
	ComponentType const&	type,
	ImageFormat const&		format,
	FilterMode const&		magFilter,
	FilterMode const&		minFilter,
	uint8* const			data,
	ComponentLayout const&	layout,
	ImageTarget const&		target
) {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("Image2D::newImage");
	#endif // MAKAILIB_DEBUG
	const auto tg = convert(target);
	image->create();
	image->bind(target);
	image->attributes = {
		width,
		height,
		type,
		format,
		layout,
		minFilter,
		magFilter
	};
	uint cl = convert(layout);
	if (!cl) cl = convert(format);
	// Bind image data
	glTexImage2D(
		tg,
		0,
		cl,
		width,
		height,
		0,
		convert(format),
		convert(type),
		data
	);
	// Set image wrapping & mipmaps
	glTexParameteri(tg, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(tg, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(tg);
	// Set filtering
	glTexParameteri(tg, GL_TEXTURE_MIN_FILTER, convert(minFilter));
	glTexParameteri(tg, GL_TEXTURE_MAG_FILTER, convert(magFilter));
	// Unbind image
	Image::unbind(target);
	// Return image
	return image;
}

void Image2D::saveImageToFile(String const& path, uint8 const quality, ImageFileType type) const {
	if (!exists()) return;
	ImageData imgdat = getData();
	uint8 channels = 0;
	switch (imgdat.format) {
		case ImageFormat::IF_D:
		case ImageFormat::IF_R:		channels = 1;	break;
		case ImageFormat::IF_DS:
		case ImageFormat::IF_RG:	channels = 2;	break;
		case ImageFormat::IF_RGB:	channels = 3;	break;
		default:
		case ImageFormat::IF_RGBA:	channels = 4;	break;
	}
	int result;
	if (type == ImageFileType::IFT_AUTO_DETECT) type = fromFileExtension(path);
	if (type == ImageFileType::IFT_INVALID)
		throw Error::InvalidValue("Invalid file type of '." + path.splitAtLast('.').back() + "'!", CTL_CPP_PRETTY_SOURCE);
	#define IMAGE2D_STBIWRITE_PARAMS path.cstr(), imgdat.width, imgdat.height, channels, imgdat.data.data()
	switch (type) {
		default:
		case ImageFileType::IFT_TGA:	result = stbi_write_tga(IMAGE2D_STBIWRITE_PARAMS);			break;
		case ImageFileType::IFT_PNG:	result = stbi_write_png(IMAGE2D_STBIWRITE_PARAMS, 0);		break;
		case ImageFileType::IFT_JPG:	result = stbi_write_jpg(IMAGE2D_STBIWRITE_PARAMS, quality);	break;
		case ImageFileType::IFT_BMP:	result = stbi_write_bmp(IMAGE2D_STBIWRITE_PARAMS);			break;
	}
	#undef IMAGE2D_STBIWRITE_PARAMS
	if (!result) {
		throw Makai::File::FileSaveError(
			"Could not save image file!"
		);
	}
	return;
}
