#ifndef MAKAILIB_GRAPH_RENDERER_DRAWABLE_H
#define MAKAILIB_GRAPH_RENDERER_DRAWABLE_H

#include "../../../compat/ctl.hpp"
#include "../material/material.hpp"
#include "../blend.hpp"
#include "../vertex.hpp"
#include "../global.hpp"
#include "mode.hpp"
#include "core.hpp"

/// @brief Graphical facilities.
namespace Makai::Graph {
	/// @brief Drawable object interface.
	class IDrawable {
	public:
		/// @brief Constructs the drawable object.
		/// @param manual Whether the object is manually rendered. By default, it is `falsie`.
		IDrawable(bool const manual = false);

		/// @brief Constructs the drawable object.
		/// @param manual Whether the object is manually rendered.
		/// @param layer Layer to register the object to.
		IDrawable(bool const manual, usize const layer);
		
		/// @brief Destructor.
		virtual ~IDrawable();

		/// @brief Sets the object as to be manually rendered.
		IDrawable& setManual();

		/// @brief Sets the object as to be automatically rendered.
		/// @param layer Layer to register the object to.
		IDrawable& setAuto(usize const renderLayer);

		/// @brief Sets the object to only render for a specific layer.
		/// @param renderLayer Layer to register the object to.
		/// @return Reference to self.
		IDrawable& setRenderLayer(usize const renderLayer);

		/// @brief Adds the object to a render layer.
		/// @param renderLayer Layer to register the object to.
		/// @return Reference to self.
		IDrawable& addToRenderLayer(usize const renderLayer);

		/// @brief Removes the object from a render layer.
		/// @param renderLayer Layer to register the object to.
		/// @return Reference to self.
		IDrawable& removeFromRenderLayer(usize const renderLayer);
		
		/// @brief Renders the object to the screen.
		void render();

		/// @brief Whether the object should render.
		bool active	= true;

	protected:
		/// @brief Draws the object to the screen. Must be implemented.
		virtual void draw() = 0;

	private:
		/// @brief Render callback wrapper.
		RenderCallback const doRender = [&] {render();};

		/// @brief Whether the object is rendered manually.
		bool manualMode = false;
	};

	/// @brief OpenGL drawable object interface.
	class IGLDrawable: public IDrawable, public Blendable {
	public:
		/// @brief Constructs the drawable object.
		/// @param layer Layer to register the object to.
		/// @param manual Whether the object is manually rendered.
		IGLDrawable(usize const layer = 0, bool const manual = false);

		/// @brief Destructor.
		virtual ~IGLDrawable();

		/// @brief Draws the object to the screen. Must be implemented.
		virtual void draw() = 0;

		/// @brief Object transform.
		Transform3D			trans;
		/// @brief Object shader.
		Shader				shader		= Shader::DEFAULT;
		/// @brief Object point size.
		Nullable<float>		pointSize	= nullptr;
		/// @brief Object line width.
		float				lineWidth	= 1.0;

	protected:
		/// @brief Displays the object to the screen.
		/// @param vertices Vertices to display.
		/// @param count Vertex count.
		/// @param culling Object face culling.
		/// @param fill Object face fill.
		/// @param mode Display mode.
		/// @param instances Instance count.
		void display(
			Vertex* const		vertices,
			usize const			count,
			CullMode const&		culling,
			FillMode const&		fill,
			DisplayMode const&	mode		= DisplayMode::ODM_TRIS,
			usize const			instances	= 1
		);

		/// @brief Prepares the object to render.
		void prepare();

		/// @brief Vertex array.
		uint vao;
		/// @brief Vertex buffer.
		uint vbo;
	};

	/// @brief Type must be an OpenGL drawable object.
	template<class T>
	concept GLDrawableType = Makai::Type::Subclass<T, IGLDrawable>;
}

#endif // MAKAILIB_GRAPH_RENDERER_DRAWABLE_H
