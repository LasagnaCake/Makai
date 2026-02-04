#ifndef MAKAILIB_GRAPH_TEXTURE_H
#define MAKAILIB_GRAPH_TEXTURE_H

#include "../../compat/ctl.hpp"
#include "../../file/json.hpp"
#include "image.hpp"

/// @brief Graphical facilites.
namespace Makai::Graph {
	/// @brief Image container.
	class Texture2D {
	public:
		/// @brief Underlying image instance.
		using Image2DInstance = Instance<Image2D>;

		/// @brief Texture wrapping mode.
		enum class WrapMode {
			/// @brief Clamp to edge.
			WM_CLAMP,
			/// @brief Linear repetition.
			WM_REPEAT,
			/// @brief Mirrored repetition.
			WM_MIRROR_REPEAT,
			/// @brief Mirror once, then clamp.
			WM_MIRROR_CLAMP
		};

		/// @brief Creates a texture from a JSON structure.
		/// @param img Strucure to create from.
		/// @param sourcepath Structure source path.
		/// @return Created texture.
		static Texture2D fromJSON(JSON::Value img, String const& sourcepath = "");

		/// @brief Default constructor.
		Texture2D(): image(new Image2D()) {}

		/// @brief Default constructor.
		Texture2D(nulltype): Texture2D() {}

		/// @brief Creates the texture.
		/// @param width Image width.
		/// @param height Image height.
		/// @param type Component type. By default, it is `CT_UBYTE`.
		/// @param format Image format. By default, it is `IF_RGBA`.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @param data Image contents. By default, it is `nullptr`.
		/// @param layout Component layout. By default, it is automatically deduced.
		Texture2D(
			uint32 const					width,
			uint32 const					height,
			Image2D::ComponentType const&	type		= Image2D::ComponentType::CT_UBYTE,
			Image2D::ImageFormat const&		format		= Image2D::ImageFormat::IF_RGBA,
			Image2D::FilterMode const&		magFilter	= Image2D::FilterMode::FM_SMOOTH,
			Image2D::FilterMode const&		minFilter	= Image2D::FilterMode::FM_SMS,
			uint8* const					data		= NULL,
			Image2D::ComponentLayout const&	layout		= Image2D::ComponentLayout::CL_AUTO
		);

		/// @brief Creates the texture from an image file.
		/// @param path Path to image file.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @throw Error::FailedAction on image load failure.
		Texture2D(
			String const& path,
			Image2D::FilterMode const& magFilter = Image2D::FilterMode::FM_SMOOTH,
			Image2D::FilterMode const& minFilter = Image2D::FilterMode::FM_SMS
		);

		/// @brief Creates the texture from an image.
		/// @param image Image to create from.
		Texture2D(
			Image2D::ImageData const& image
		);

		/// @brief Instances an image from another texture.
		/// @param other Texture to get instance from.
		Texture2D(
			Texture2D const& other
		);

		/// @brief Creates the texture from a section in another texture.
		/// @param other `Texture2D` to copy from.
		/// @param startX Starting X position.
		/// @param startY Starting Y position.
		/// @param endX Ending X position.
		/// @param endY Ending Y position.
		/// @param filter Whether to apply a linear filter to the image. By default, it is `false`.
		Texture2D(
			Texture2D const& other,
			uint32 const startX,
			uint32 const startY,
			uint32 const endX,
			uint32 const endY,
			bool const filter = false
		);

		/// @brief Creates the texture.
		/// @param width Image width.
		/// @param height Image height.
		/// @param type Component type. By default, it is `CT_UBYTE`.
		/// @param format Image format. By default, it is `IF_RGBA`.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @param data Image contents. By default, it is `nullptr`.
		/// @param layout Component layout. By default, it is automatically deduced.
		/// @return Reference to self.
		/// @note Does nothing if underlying image already exists.
		Texture2D& create(
			uint32 const					width,
			uint32 const					height,
			Image2D::ComponentType const&	type		= Image2D::ComponentType::CT_UBYTE,
			Image2D::ImageFormat const&		format		= Image2D::ImageFormat::IF_RGBA,
			Image2D::FilterMode const&		magFilter	= Image2D::FilterMode::FM_SMOOTH,
			Image2D::FilterMode const&		minFilter	= Image2D::FilterMode::FM_SMS,
			uint8* const					data		= NULL,
			Image2D::ComponentLayout const&	layout		= Image2D::ComponentLayout::CL_AUTO
		);

		/// @brief Creates the texture from an image file.
		/// @param path Path to image file.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @return Reference to self.
		/// @throw Error::FailedAction on image load failure.
		/// @note Does nothing if underlying image already exists.
		Texture2D& create(
			String const& path,
			Image2D::FilterMode const& magFilter = Image2D::FilterMode::FM_SMOOTH,
			Image2D::FilterMode const& minFilter = Image2D::FilterMode::FM_SMS
		);

		/// @brief Creates the texture from an image.
		/// @param image Image to create from.
		/// @return Reference to self.
		/// @note Does nothing if underlying image already exists.
		Texture2D& create(
			Image2D::ImageData const& image
		);

		/// @brief Instances the image from another texture.
		/// @param other Texture to get instance from.
		/// @return Reference to self.
		/// @note Does nothing if underlying image already exists.
		Texture2D& create(
			Texture2D const& other
		);

		/// @brief Creates the texture from a section in another texture.
		/// @param other `Texture2D` to copy from.
		/// @param startX Starting X position.
		/// @param startY Starting Y position.
		/// @param endX Ending X position.
		/// @param endY Ending Y position.
		/// @param filter Whether to apply a linear filter to the image. By default, it is `false`.
		/// @return Reference to self.
		/// @note Does nothing if underlying image already exists.
		Texture2D& create(
			Texture2D const& other,
			uint32 const startX,
			uint32 const startY,
			uint32 const endX,
			uint32 const endY,
			bool const filter = false
		);

		/// @brief Unbinds the underlying image instance.
		/// @return Reference to self.
		Texture2D& destroy();

		/// @brief Destroys the underlying image, leaving it blank.
		/// @return Reference to self.
		Texture2D& clear();

		/// @brief Creates the texture.
		/// @param width Image width.
		/// @param height Image height.
		/// @param type Component type. By default, it is `CT_UBYTE`.
		/// @param format Image format. By default, it is `IF_RGBA`.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @param data Image contents. By default, it is `nullptr`.
		/// @param layout Component layout. By default, it is automatically deduced.
		/// @return Reference to self.
		Texture2D& make(
			uint32 const					width,
			uint32 const					height,
			Image2D::ComponentType const&	type		= Image2D::ComponentType::CT_UBYTE,
			Image2D::ImageFormat const&		format		= Image2D::ImageFormat::IF_RGBA,
			Image2D::FilterMode const&		magFilter	= Image2D::FilterMode::FM_SMOOTH,
			Image2D::FilterMode const&		minFilter	= Image2D::FilterMode::FM_SMS,
			uint8* const					data		= NULL,
			Image2D::ComponentLayout const&	layout		= Image2D::ComponentLayout::CL_AUTO
		);

		/// @brief Creates the texture from an image file.
		/// @param path Path to image file.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @return Reference to self.
		/// @throw Error::FailedAction on image load failure.
		Texture2D& make(
			String const& path,
			Image2D::FilterMode const& magFilter = Image2D::FilterMode::FM_SMOOTH,
			Image2D::FilterMode const& minFilter = Image2D::FilterMode::FM_SMS
		);

		/// @brief Creates the texture from an image.
		/// @param image Image to create from.
		/// @return Reference to self.
		Texture2D& make(
			Image2D::ImageData const& image
		);

		/// @brief Instances the image from another texture.
		/// @param other Texture to get instance from.
		/// @return Reference to self.
		Texture2D& make(
			Texture2D const& other
		);

		/// @brief Creates the texture from a section in another texture.
		/// @param other Texture to copy from.
		/// @param startX Starting X position.
		/// @param startY Starting Y position.
		/// @param endX Ending X position.
		/// @param endY Ending Y position.
		/// @param filter Whether to apply a linear filter to the image. By default, it is `false`.
		/// @return Reference to self.
		Texture2D& make(
			Texture2D const& other,
			uint32 const startX,
			uint32 const startY,
			uint32 const endX,
			uint32 const endY,
			bool const filter = false
		);

		/// @brief Creates a new copy of the underlying image, and binds itself to it.
		/// @param filter Whether to apply a linear filter to the image.
		/// @return Reference to self.
		Texture2D& makeUnique(bool const filter = false);

		/// @brief Copy assignment operator.
		/// @param other `Texture2D` to get instance from.
		/// @return Reference to self.
		Texture2D&			operator=(Texture2D const& other);
		/// @brief Move assignment operator.
		/// @param other `Texture2D` to move.
		/// @return Reference to self.
		Texture2D&			operator=(Texture2D&& other);
		/// @brief Unbinds the underlying image instance.
		/// @param . Ignored.
		/// @return Reference to self.
		inline Texture2D&	operator=(nulltype) {destroy(); return *this;}

		/// @brief Equality comparison operator.
		/// @param other `Texture2D` to compare with.
		/// @return Whether they're equal.
		bool operator==(Texture2D const& other) const;
		/// @brief Threeway comparison operator.
		/// @param other `Texture2D` to compare with.
		/// @return Order between objects.
		ValueOrder operator<=>(Texture2D const& other) const;

		/// @brief Copies a texture's image into this texture's image.
		/// @param other Texture to copy from.
		/// @param startX Starting X position.
		/// @param startY Starting Y position.
		/// @param endX Ending X position.
		/// @param endY Ending Y position.
		/// @param filter Whether to apply a linear filter to the image. By default, it is `false`.
		/// @return Reference to self.
		Texture2D& copyFrom(
			Texture2D const& other,
			uint32 const startX,
			uint32 const startY,
			uint32 const endX,
			uint32 const endY,
			bool const filter = false
		);

		/// @brief Copies a texture's image into this texture's image.
		/// @param other Texture to copy from.
		/// @param filter Whether to apply a linear filter to the image. By default, it is `false`.
		/// @return Reference to self.
		Texture2D& copyFrom(
			Texture2D const& other,
			bool const filter = false
		);

		/// @brief Sets the texture's wrap mode.
		/// @param horizontal Horizontal wrap mode.
		/// @param vertical Vertical wrap mode.
		/// @return Reference to self.
		Texture2D& setWrapMode(
			WrapMode const& horizontal,
			WrapMode const& vertical
		);

		/// @brief Sets the texture's wrap mode.
		/// @param mode Horizontal and vertical wrap mode. By default, it is `WM_REPEAT`.
		/// @return Reference to self.
		Texture2D& setWrapMode(WrapMode const& mode = WrapMode::WM_REPEAT);

		/// @brief Sets the texture's filtering.
		/// @param magFilter Magnifying filter. By default, it is `FM_SMOOTH`.
		/// @param minFilter Minifying filter. By default, it is `FM_SMS`.
		/// @return Reference to self.
		Texture2D& setFilterMode(
			Image2D::FilterMode const& magFilter = Image2D::FilterMode::FM_SMOOTH,
			Image2D::FilterMode const& minFilter = Image2D::FilterMode::FM_SMS
		);

		/// @brief Returns the minifying filter.
		/// @return Minifying filter.
		Image2D::FilterMode minFilter() const;

		/// @brief Returns the magnifying filter.
		/// @return Magnifying filter.
		Image2D::FilterMode magFilter() const;

		/// @brief Returns the underlying image's attributes.
		/// @return Image attributes.
		Image2D::Attributes attributes() const;

		/// @brief Enables the underlying image to be used in the shader.
		/// @param slot Image slot to bind to. By default, it is slot zero.
		/// @return Reference to self.
		Texture2D& enable(uint8 const slot = 0);

		/// @brief Enables the underlying image to be used in the shader.
		/// @param slot Image slot to bind to. By default, it is slot zero.
		/// @return Const reference to self.
		Texture2D const& enable(uint8 const slot = 0) const;

		/// @brief Enables the underlying image to be used in the shader.
		/// @param slot Image slot to bind to. By default, it is slot zero.
		/// @return Reference to self.
		Texture2D&			operator()(uint8 const slot = 0);
		/// @brief Enables the underlying image to be used in the shader.
		/// @param slot Image slot to bind to. By default, it is slot zero.
		/// @return Const reference to self.
		Texture2D const&	operator()(uint8 const slot = 0) const;

		/// @brief Returns the underlying image's ID.
		/// @return Image ID.
		uint32 getID() const;

		/// @brief Returns the underlying image's contents.
		/// @return Image contents.
		Image2D::ImageData getData() const;

		/// @brief Saves the underlying image to disk.
		/// @param path Path to save image to.
		/// @param quality Image quality.
		/// @param type Image file type. By default, it is automatically detected.
		/// @return Const reference to self.
		Texture2D const& saveToFile(
			String const& path,
			uint8 const quality,
			Image2D::ImageFileType const& type = Image2D::ImageFileType::IFT_AUTO_DETECT
		) const;

		/// @brief Saves the underlying image to disk.
		/// @param path Path to save image to.
		/// @param quality Image quality.
		/// @param type Image file type. By default, it is automatically detected.
		/// @return Reference to self.
		Texture2D& saveToFile(
			String const& path,
			uint8 const quality,
			Image2D::ImageFileType const& type = Image2D::ImageFileType::IFT_AUTO_DETECT
		);

		/// @brief Saves the underlying image to disk.
		/// @param path Path to save image to.
		/// @param type Image file type. By default, it is automatically detected.
		/// @return Const reference to self.
		Texture2D const& saveToFile(
			String const& path,
			Image2D::ImageFileType const& type = Image2D::ImageFileType::IFT_AUTO_DETECT
		) const;

		/// @brief Saves the underlying image to disk.
		/// @param path Path to save image to.
		/// @param type Image file type. By default, it is automatically detected.
		/// @return Reference to self.
		Texture2D& saveToFile(
			String const& path,
			Image2D::ImageFileType const& type = Image2D::ImageFileType::IFT_AUTO_DETECT
		);

		/// @brief Returns whether there is an image bound to this texture, and that image isn't empty.
		/// @return Whether image exists.
		bool exists() const;

		/// @brief Returns whether there is an image bound to this texture, and that image isn't empty.
		operator bool() const;

	private:
		/// @brief Image bound to texture.
		Image2DInstance image;
	};
}

#endif // MAKAILIB_GRAPH_TEXTURE_H
