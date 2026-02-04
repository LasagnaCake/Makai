#ifndef MAKAILIB_GRAPH_RENDERER_FRAMEBUFFER_H
#define MAKAILIB_GRAPH_RENDERER_FRAMEBUFFER_H

#include "../texture.hpp"
#include "../vertex.hpp"
#include "../../color/color.hpp"
#include "../blend.hpp"
#include "../shader/shader.hpp"
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
			uint id			= 0;
			/// @brief Buffer width.
			uint32 width	= 0;
			/// @brief Buffer height.
			uint32 height	= 0;
		};

		/// @brief Basic screen buffer.
		class ABuffer: private BufferObject {
		public:
			/// @brief Default constructor.
			ABuffer() {}

			/// @brief Creates the buffer with a given width and height.
			/// @param width Buffer width.
			/// @param height Buffer height.
			ABuffer(
				uint32 const width,
				uint32 const height
			);

			/// @brief Destructor.
			virtual ~ABuffer();

			/// @brief Destroys the buffer.
			/// @return Reference to self.
			virtual ABuffer& destroy();

			/// @brief Creates the buffer with a given width and height.
			/// @param width Buffer width.
			/// @param height Buffer height.
			virtual ABuffer& create(
				uint32 const width,
				uint32 const height
			);

			/// @brief Renders the buffer to another buffer.
			/// @param target Buffer to render to.
			/// @return Reference to self.
			ABuffer& render(ABuffer const& target);

			/// @brief Renders the buffer to another buffer.
			/// @param target Buffer to render to.
			/// @return Reference to self.
			virtual ABuffer& renderTo(BufferObject const& target) = 0;

			/// @brief Enables the buffer.
			/// @return Reference to self.
			virtual ABuffer& enable();

			/// @brief Enables the buffer.
			/// @return Reference to self.
			ABuffer& operator()();

			/// @brief Disables the buffer.
			/// @return Reference to self.
			virtual ABuffer& disable();

			/// @brief Returns whether the buffer exists.
			/// @return Whether buffer exists.
			bool exists() const;

			/// @brief Returns the buffer as a buffer object.
			/// @return Buffer as buffer object.
			BufferObject data() const;

		protected:
			/// @brief Returns the buffer's width.
			/// @return Buffer width.
			uint32 getWidth() const;
			/// @brief Returns the buffer's height.
			/// @return Buffer height.
			uint32 getHeight() const;
			/// @brief Returns the buffer's underlying API ID.
			/// @return Underlying API ID.
			uint32 getID() const;

		private:
			/// @brief Whether buffer exists.
			bool created = false;
		};
	}

	/// @brief Render operation buffer.
	class DrawBuffer: public Base::ABuffer, public Blendable {
	public:
		/// @brief Buffer storage.
		struct Storage {
			/// @brief Color buffer.
			Texture2D screen;
			/// @brief Normal buffer.
			Texture2D normal;
			/// @brief Position buffer.
			Texture2D position;
			/// @brief Depth buffer.
			Texture2D depth;
		};

		/// @brief Default constructor.
		DrawBuffer(): Blendable() {}

		/// @brief Creates the buffer with a given width and height.
		/// @param width Buffer width.
		/// @param height Buffer height.
		DrawBuffer(
			uint32 const width,
			uint32 const height
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
			uint32 const width,
			uint32 const height
		) override;

		/// @brief Enables the buffer.
		/// @return Reference to self.
		DrawBuffer& enable() override;

		/// @brief Clears both the color and the depth buffers.
		/// @return Reference to self.
		DrawBuffer& clearBuffers();

		/// @brief Clears the color buffer.
		/// @return Reference to self.
		DrawBuffer& clearColorBuffer();

		/// @brief Clears the depth buffer.
		/// @return Reference to self.
		DrawBuffer& clearDepthBuffer();

		/// @brief Disables the buffer.
		/// @return Reference to self.
		DrawBuffer& disable() override;

		/// @brief Transfromation.
		Transform3D trans;
		/// @brief UV transformation.
		Transform3D uv;
		/// @brief Buffer shape.
		Vertex rect[4];
		/// @brief Buffer shader.
		Shader shader;
		/// @brief  Screen Vertex Unit space. Usually the inverse of the camera's orthographic size.
		Vector2 screenVUSpace = 1;

		/// @brief Returns the buffer storage.
		/// @return Buffer storage.
		Storage const& storage() const {return buffer;}

		/// @brief Renders the buffer to another buffer.
		/// @param target Buffer to render to.
		/// @return Reference to self.
		DrawBuffer& renderTo(Base::BufferObject const& target) override;

	protected:
		/// @brief Buffer clear color.
		Vector4 clearColor = Color::CLEAR;

	private:
		/// @brief Buffer storage.
		Storage buffer;
		/// @brief Vertex array.
		uint vao;
		/// @brief Vertex buffer.
		uint vbo;
	};

	/// @brief Frame buffer.
	class FrameBuffer: public DrawBuffer {
	public:
		using DrawBuffer::DrawBuffer;

		/// @brief Material to use.
		Material::BufferMaterial material;

		/// @brief Renders the buffer to another buffer.
		/// @param target Buffer to render to.
		/// @return Reference to self.
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
