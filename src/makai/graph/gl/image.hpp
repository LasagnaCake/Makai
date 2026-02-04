#ifndef MAKAILIB_GRAPH_IMAGE_H
#define MAKAILIB_GRAPH_IMAGE_H

#include "../../compat/ctl.hpp"
#include "../../file/file.hpp"
#include "core.hpp"

/// @brief Graphical facilites.
namespace Makai::Graph {
	struct Texture2D;

	/// @brief Basic image.
	struct Image {
		/// @brief Image type.
		enum class ImageTarget {
			IT_TEXTURE_2D
		};

		/// @brief Default constructor.
		Image() {}

		/// @brief Destructor.
		~Image() {destroy();}

		/// @brief Binds the image to be manipulated.
		/// @param target Image type to bind to.
		/// @return Reference to self.
		Image&			bind(ImageTarget const& target = ImageTarget::IT_TEXTURE_2D);
		/// @brief Binds the image to be manipulated.
		/// @param target Image type to bind to.
		/// @return Const reference to self.
		Image const&	bind(ImageTarget const& target = ImageTarget::IT_TEXTURE_2D) const;

		/// @brief Destroys the image.
		/// @return Reference to self.
		Image& destroy();

		/// @brief Creates the image, if it doesn't exist.
		/// @return Reference to self.
		Image& create();

		/// @brief Creates the image.
		/// @return Reference to self.
		Image& make();

		/// @brief Unbinds the currently bound image.
		/// @param target Image type to unbind.
		static void unbind(ImageTarget const& target = ImageTarget::IT_TEXTURE_2D);

		/// @brief Enables the image to be used in the shader.
		/// @param slot Image slot to bind to. By default, it is slot zero.
		/// @param target Image type to enable. By default, it is `IT_TEXTURE_2D`.
		/// @return Reference to self.
		Image&			use(uint8 const slot = 0, ImageTarget const& target = ImageTarget::IT_TEXTURE_2D);
		/// @brief Enables the image to be used in the shader.
		/// @param slot Image slot to bind to. By default, it is slot zero.
		/// @param target Image type to enable. By default, it is `IT_TEXTURE_2D`.
		/// @return Const reference to self.
		Image const&	use(uint8 const slot = 0, ImageTarget const& target = ImageTarget::IT_TEXTURE_2D) const;

		/// @brief Enables the image to be used in the shader.
		/// @param slot Image slot to bind to. By default, it is slot zero.
		/// @param target Image type to enable. By default, it is `IT_TEXTURE_2D`.
		/// @return Reference to self.
		Image&			operator()(uint8 const slot = 0, ImageTarget const& target = ImageTarget::IT_TEXTURE_2D);
		/// @brief Enables the image to be used in the shader.
		/// @param slot Image slot to bind to. By default, it is slot zero.
		/// @param target Image type to enable. By default, it is `IT_TEXTURE_2D`.
		/// @return Const reference to self.
		Image const&	operator()(uint8 const slot = 0, ImageTarget const& target = ImageTarget::IT_TEXTURE_2D) const;

		/// @brief Enables an image to be used in the shader.
		/// @param image Image ID.
		/// @param slot Image slot to bind to.
		/// @param target Image type to enable. By default, it is `IT_TEXTURE_2D`.
		static void set(
			uint32 const		image,
			uint8 const			slot,
			ImageTarget const&	target = ImageTarget::IT_TEXTURE_2D
		);

		/// @brief Equality comparison operator.
		/// @param other `Image` to compare to.
		/// @return Whether they're equal.
		bool operator==(Image const& other) const;
		/// @brief Threeway comparison operator.
		/// @param other `Image` to compare to.
		/// @return Order between objects.
		ValueOrder operator<=>(Image const& other) const;

		/// @brief Returns the image's ID.
		/// @return Image ID.
		explicit operator uint32() const;
		/// @brief Returns the image's ID.
		/// @return Image ID.
		uint32 getID() const;

		/// @brief Returns whether the image exists.
		/// @return Whether image exists.
		bool exists() const;
		/// @brief Returns whether the image exists.
		/// @return Whether image exists.
		operator bool() const;

	private:
		/// @brief Image ID.
		uint id = 0;
	};

	/// @brief 2D image.
	struct Image2D: public Image {
		using Image::create;

		/// @brief File type.
		enum class ImageFileType {
			IFT_INVALID = -1,
			IFT_AUTO_DETECT,
			/// @brief Texture Grid Array.
			IFT_TGA,
			/// @brief Portable Network Graphics.
			IFT_PNG,
			/// @brief Just Poor Graphics.
			IFT_JPG,
			/// @brief Windows Bitmap.
			IFT_BMP
		};

		/// @brief Component type.
		enum class ComponentType {
			/// @brief 8-bit unsigned integer.
			CT_UBYTE,
			/// @brief 8-bit signed integer.
			CT_BYTE,
			/// @brief 16-bit unsigned integer.
			CT_USHORT,
			/// @brief 16-bit signed integer.
			CT_SHORT,
			/// @brief 16-bit floating point number.
			CT_HALF_FLOAT,
			/// @brief 32-bit unsigned integer.
			CT_UINT,
			/// @brief 32-bit signed integer.
			CT_INT,
			/// @brief 32-bit floating point number.
			CT_FLOAT,
			/// @brief 32-bit special integer. For depth & stencil buffers.
			CT_UINT_24_8
		};

		/// @brief Image format.
		enum class ImageFormat {
			/// @brief Depth-only.
			IF_D,
			/// @brief Red-only.
			IF_R,
			/// @brief Depth-Stencil.
			IF_DS,
			/// @brief Red-Green.
			IF_RG,
			/// @brief Red-Green-Blue.
			IF_RGB,
			/// @brief Red-Green-Blue-Alpha.
			IF_RGBA,
		};

		/// @brief Component layout.
		enum class ComponentLayout {
			/// @brief Auto-detect components.
			CL_AUTO = -1,
			/// @brief Depth-only layout.
			CL_D,
			/// @brief Red-only layout.
			CL_R,
			/// @brief Depth-Stencil layout.
			CL_DS,
			/// @brief Red-Green layout.
			CL_RG,
			/// @brief Red-Green-Blue layout.
			CL_RGB,
			/// @brief Red-Green-Blue-Alpha layout.
			CL_RGBA,
			/// @brief RGBA layout with 16-bit floating point numbers.
			CL_RGBA_16F,
			/// @brief 24-bit depth, 8-bit stencil layout.
			CL_D24_S8
		};

		/// @brief Filter mode.
		enum class FilterMode {
			/// @brief Nearest pixel to UV.
			FM_NEAREST,
			/// @brief Linear interpolation between pixels.
			FM_SMOOTH,
			/// @brief Nearest mipmap level, nearest pixel to UV.
			FM_NMN,
			/// @brief Nearest mipmap level, linear interpolation between pixels.
			FM_NMS,
			/// @brief Linear interpolation between mipmap levels, nearest pixel to UV.
			FM_SMN,
			/// @brief Linear interpolation between mipmap levels, linear interpolation between pixels.
			FM_SMS,
		};

		/// @brief Image attributes.
		struct Attributes {
			/// @brief Width.
			uint32 width;
			/// @brief Height.
			uint32 height;
			/// @brief Component type.
			ComponentType	type;
			/// @brief Image format.
			ImageFormat		format;
			/// @brief Component layout.
			ComponentLayout	layout;
			/// @brief Minifying filter.
			FilterMode		minFilter;
			/// @brief Magnifying filter.
			FilterMode		magFilter;
		};

		/// @brief Image contents.
		struct ImageData: Attributes {
			/// @brief Contents.
			List<ubyte> data;
		};

		/// @brief Creates the image, if it doesn't already exist.
		/// @param width Image width.
		/// @param height Image height.
		/// @param type Component type. By default, it is `CT_UBYTE`.
		/// @param format Image format. By default, it is `IF_RGBA`.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @param data Image contents. By default, it is `nullptr`.
		/// @param layout Component layout. By default, it is automatically deduced.
		/// @return Reference to self.
		Image2D& create(
			uint32 const			width,
			uint32 const			height,
			ComponentType const&	type		= ComponentType::CT_UBYTE,
			ImageFormat const&		format		= ImageFormat::IF_RGBA,
			FilterMode const&		magFilter	= FilterMode::FM_SMOOTH,
			FilterMode const&		minFilter	= FilterMode::FM_SMS,
			uint8* const			data		= nullptr,
			ComponentLayout const&	layout		= ComponentLayout::CL_AUTO
		);

		/// @brief Creates the image.
		/// @param width Image width.
		/// @param height Image height.
		/// @param type Component type. By default, it is `CT_UBYTE`.
		/// @param format Image format. By default, it is `IF_RGBA`.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @param data Image contents. By default, it is `nullptr`.
		/// @param layout Component layout. By default, it is automatically deduced.
		/// @return Reference to self.
		Image2D& make(
			uint32 const			width,
			uint32 const			height,
			ComponentType const&	type		= ComponentType::CT_UBYTE,
			ImageFormat const&		format		= ImageFormat::IF_RGBA,
			FilterMode const&		magFilter	= FilterMode::FM_SMOOTH,
			FilterMode const&		minFilter	= FilterMode::FM_SMS,
			uint8* const			data		= nullptr,
			ComponentLayout const&	layout		= ComponentLayout::CL_AUTO
		);

		/// @brief Returns the image's attributes.
		/// @return Image attributes.
		Attributes getAttributes() const;

		/// @brief Returns the image's contents.
		/// @return Image contents.
		ImageData getData() const;

		/// @brief Saves the image to disk.
		/// @param path Path to save image to.
		/// @param quality Image quality.
		/// @param type Image file type. By default, it is automatically detected.
		/// @return Reference to self.
		Image2D&		saveToFile(
			String const&			path,
			uint8 const				quality,
			ImageFileType const&	type = ImageFileType::IFT_AUTO_DETECT
		);

		/// @brief Saves the image to disk.
		/// @param path Path to save image to.
		/// @param quality Image quality.
		/// @param type Image file type. By default, it is automatically detected.
		/// @return Const reference to self.
		Image2D const&	saveToFile(
			String const&			path,
			uint8 const				quality,
			ImageFileType const&	type = ImageFileType::IFT_AUTO_DETECT
		) const;

		/// @brief Saves the image to disk.
		/// @param path Path to save image to.
		/// @param type Image file type. By default, it is automatically detected.
		/// @return Reference to self.
		Image2D&		saveToFile(
			String const&			path,
			ImageFileType const&	type = ImageFileType::IFT_AUTO_DETECT
		);

		/// @brief Saves the image to disk.
		/// @param path Path to save image to.
		/// @param type Image file type. By default, it is automatically detected.
		/// @return Const reference to self.
		Image2D const&	saveToFile(
			String const&			path,
			ImageFileType const&	type = ImageFileType::IFT_AUTO_DETECT
		) const;

		/// @brief Creates an image in the heap.
		/// @param width Image width.
		/// @param height Image height.
		/// @param type Component type. By default, it is `CT_UBYTE`.
		/// @param format Image format. By default, it is `IF_RGBA`.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @param data Image contents. By default, it is `nullptr`.
		/// @param layout Component layout. By default, it is automatically deduced.
		/// @param target Image type to create.
		/// @return Created image.
		[[nodiscard]]
		static Image2D* newImage(
			uint32 const			width,
			uint32 const			height,
			ComponentType const&	type		= ComponentType::CT_UBYTE,
			ImageFormat const&		format		= ImageFormat::IF_RGBA,
			FilterMode const&		magFilter	= FilterMode::FM_SMOOTH,
			FilterMode const&		minFilter	= FilterMode::FM_SMS,
			uint8* const			data		= NULL,
			ComponentLayout const&	layout		= ComponentLayout::CL_AUTO,
			ImageTarget const&		target		= ImageTarget::IT_TEXTURE_2D
		);

		/// @brief Creates an image.
		/// @param image Image to create in.
		/// @param width Image width.
		/// @param height Image height.
		/// @param type Component type. By default, it is `CT_UBYTE`.
		/// @param format Image format. By default, it is `IF_RGBA`.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @param data Image contents. By default, it is `nullptr`.
		/// @param layout Component layout. By default, it is automatically deduced.
		/// @param target Image type to create.
		/// @return Passed pointer.
		static Image2D* newImage(
			Image2D* const			image,
			uint32 const			width,
			uint32 const			height,
			ComponentType const&	type		= ComponentType::CT_UBYTE,
			ImageFormat const&		format		= ImageFormat::IF_RGBA,
			FilterMode const&		magFilter	= FilterMode::FM_SMOOTH,
			FilterMode const&		minFilter	= FilterMode::FM_SMS,
			uint8* const			data		= NULL,
			ComponentLayout const&	layout		= ComponentLayout::CL_AUTO,
			ImageTarget const&		target		= ImageTarget::IT_TEXTURE_2D
		);

	private:
		void saveImageToFile(
			String const&	path,
			uint8 const		quality,
			ImageFileType	type
		) const;

		friend class Texture2D;

		/// @brief Image attributes.
		Attributes attributes;
	};
}

#endif // MAKAILIB_GRAPH_IMAGE_H
