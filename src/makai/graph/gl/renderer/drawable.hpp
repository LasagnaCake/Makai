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
	class ADrawable: IVisible, IServerEntity {
	public:
		/// @brief Constructs the drawable object.
		/// @param manual Whether the object is manually rendered. By default, it is `falsie`.
		ADrawable(bool const manual = false);

		/// @brief Constructs the drawable object.
		/// @param manual Whether the object is manually rendered.
		/// @param layer Layer to register the object to.
		ADrawable(bool const manual, usize const layer);

		/// @brief Destructor.
		virtual ~ADrawable();

		/// @brief Sets the object as to be manually rendered.
		ADrawable& setManual();

		/// @brief Sets the object as to be automatically rendered.
		/// @param layer Layer to register the object to.
		ADrawable& setAuto(usize const renderLayer);

		/// @brief Sets the object to only render for a specific layer.
		/// @param renderLayer Layer to register the object to.
		/// @return Reference to self.
		ADrawable& setRenderLayer(usize const renderLayer);

		/// @brief Adds the object to a render layer.
		/// @param renderLayer Layer to register the object to.
		/// @return Reference to self.
		ADrawable& addToRenderLayer(usize const renderLayer);

		/// @brief Removes the object from a render layer.
		/// @param renderLayer Layer to register the object to.
		/// @return Reference to self.
		ADrawable& removeFromRenderLayer(usize const renderLayer);

		/// @brief Renders the object to the screen.
		void render();

		/// @brief Whether the object should render.
		bool active	= true;

		void show() override {active = true;	}
		void hide() override {active = false;	}

	protected:
		/// @brief Draws the object to the screen. Must be implemented.
		virtual void draw() = 0;

	private:
		void doRender() override final;

		/// @brief Whether the object is rendered manually.
		bool manualMode = false;
	};

	/// @brief Graphic API drawable object interface.
	class AGraphic: public ADrawable, public Blendable {
	public:
		/// @brief Constructs the drawable object.
		/// @param layer Layer to register the object to.
		/// @param manual Whether the object is manually rendered.
		AGraphic(usize const layer = 0, bool const manual = false);

		/// @brief Destructor.
		virtual ~AGraphic();

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

	private:
		/// @brief Vertex array.
		uint vao;
		/// @brief Vertex buffer.
		uint vbo;
	};

	/// @brief Type must be a graphic object.
	template<class T>
	concept GraphicType = Makai::Type::Subclass<T, AGraphic>;
}

#endif // MAKAILIB_GRAPH_RENDERER_DRAWABLE_H
