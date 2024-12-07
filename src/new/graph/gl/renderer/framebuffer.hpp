#ifndef MAKAILIB_GRAPH_RENDERER_FRAMEBUFFER_H
#define MAKAILIB_GRAPH_RENDERER_FRAMEBUFFER_H

#include "../texture.hpp"
#include "../vertex.hpp"
#include "../shader.hpp"
#include "../color.hpp"
#include "../blend.hpp"
#include "../material/material.hpp"

/*
	Every single time this code gets touched,
	it becomes worse and worse.
*/

/// @brief Graphical facilities.
namespace Makai::Graph {
	/// @brief Base classes.
	namespace Base {
		/// @brief Basic buffer object.
		struct BufferObject {
			/// @brief Underlying API ID.
			uint id		= 0;
			/// @brief Buffer width.
			uint width	= 0;
			/// @brief Buffer height.
			uint height	= 0;
		};

		/// @brief Basic screen buffer.
		class Buffer: private BufferObject {
		public:
			/// @brief Empty constructor.
			Buffer() {}

			/// @brief Creates the buffer with a given width and height.
			/// @param width Buffer width.
			/// @param height Buffer height.
			Buffer(
				uint const width,
				uint const height
			);

			/// @brief Destructor.
			virtual ~Buffer();
			
			/// @brief Destroys the buffer.
			/// @return Reference to self.
			virtual Buffer& destroy();
			
			/// @brief Creates the buffer with a given width and height.
			/// @param width Buffer width.
			/// @param height Buffer height.
			virtual Buffer& create(
				uint const width,
				uint const height
			);
			
			/// @brief Renders the buffer to another buffer.
			/// @param target Buffer to render to.
			/// @return Reference to self.
			Buffer& render(Buffer const& target);

			/// @brief Renders the buffer to another buffer.
			/// @param target Buffer to render to.
			/// @return Reference to self.
			virtual Buffer& renderTo(BufferObject const& target) = 0;

			/// @brief Enables the buffer.
			/// @return Reference to self.
			virtual Buffer& enable();

			/// @brief Enables the buffer.
			/// @return Reference to self.
			Buffer& operator()();

			/// @brief Disables the buffer.
			/// @return Reference to self.
			virtual Buffer& disable();

			/// @brief Returns whether the buffer exists.
			/// @return Whether buffer exists.
			bool exists() const;
			
			/// @brief Returns the buffer as a buffer object.
			/// @return Buffer as buffer object.
			BufferObject data() const;

		protected:
			/// @brief Returns the buffer's width.
			/// @return Buffer width.
			uint getWidth() const;
			/// @brief Returns the buffer's height.
			/// @return Buffer height.
			uint getHeight() const;
			/// @brief Returns the buffer's underlying API ID.
			/// @return Underlying API ID.
			uint getID() const;

		private:
			/// @brief Whether buffer exists.
			bool created = false;
		};
	}

	/// @brief Basic frame buffer.
	class DrawBuffer: public Base::Buffer, public Blendable {
	public:
		/// @brief Empty constructor.
		DrawBuffer(): Blendable() {}

		/// @brief Creates the buffer with a given width and height.
		/// @param width Buffer width.
		/// @param height Buffer height.
		DrawBuffer(
			uint const width,
			uint const height
		);

		/// @brief Destructor.
		virtual ~DrawBuffer();

		/// @brief Destroys the buffer.
		/// @return Reference to self.
		DrawBuffer& destroy() override;

		/// @brief Creates the buffer with a given width and height.
		/// @param width Buffer width.
		/// @param height Buffer height.
		DrawBuffer& create(
			uint const width,
			uint const height
		) override;

		DrawBuffer& enable() override;

		DrawBuffer& clearBuffers();

		DrawBuffer& clearColorBuffer();

		DrawBuffer& clearDepthBuffer();

		DrawBuffer& disable() override;

		/// The framebuffer's vertex transformation.
		Transform3D trans;
		/// The framebuffer's UV transformation.
		Transform3D uv;
		/// The framebuffer's shape.
		Vertex rect[4];
		/// The framebuffer's rendering shader.
		Shader shader;
		/// The framebuffer's screen Vertex Unit space. Usually the inverse of the camera's orthographic size.
		Vector2 screenVUSpace = 1;
		/// The framebuffer's blend function & equation setting.
		BlendMode blend;

		Texture2D getScreenBuffer() const;
		Texture2D getDepthBuffer() const;

		DrawBuffer& renderTo(Base::BufferObject const& target) override;

	protected:
		Vector4 clearColor = Color::CLEAR;

	private:
		struct {
			Texture2D screen;
			Texture2D depth;
		} buffer;
		uint vao;
		uint vbo;
	};

	class FrameBuffer: public DrawBuffer {
	public:
		using DrawBuffer::DrawBuffer;

		Material::BufferMaterial material;
	
		FrameBuffer& renderTo(Base::BufferObject const& target) override final {
			if (!exists()) return *this;
			// Set material data
			clearColor = material.background;
			material.use(shader);
			// Render buffer
			DrawBuffer::renderTo(target);
			return *this;
		}
	};
}

#endif // MAKAILIB_GRAPH_RENDERER_FRAMEBUFFER_H
