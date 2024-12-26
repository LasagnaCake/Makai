#ifndef MAKAILIB_GRAPH_RENDERER_BAR_H
#define MAKAILIB_GRAPH_RENDERER_BAR_H

#include "../../../compat/ctl.hpp"
#include "drawable.hpp"
#include "text.hpp"

#ifndef RADIAL_BAR_RESOLUTION
#define RADIAL_BAR_RESOLUTION 12
#endif // RADIAL_BAR_RESOLUTION

/// @brief Graphical facilities.
namespace Makai::Graph {
	/// @brief Base classes.
	namespace Base {
		/// @brief Basic progressbar contents.
		struct Progressbar {
			/// @brief Bar-dependent UV shift.
			float uvShift		= 0;
			/// @brief Current value.
			float value			= 0;
			/// @brief Maximum value.
			float max			= 100;
			/// @brief Size.
			Vector2	size		= 1;
			/// @brief UV scale.
			Vector2	uvScale		= 1;
			/// @brief Whether the UV should adapt to the bar's progress.
			bool 	dynamicUV	= true;
		};
	}

	/// @brief Type must be a valid bar type.
	template<class T>
	concept BarType = Makai::Type::Subclass<T, Base::Progressbar> && Makai::Type::Subclass<T, IGLDrawable>;

	/// @brief Linear progress bar.
	class LinearBar: public IGLDrawable, public Base::Progressbar {
	public:
		/// @brief Constructs the progressbar.
		/// @param layer Layer to register the object to. By default, it is layer zero.
		/// @param manual Whether the object is manually rendered. By default, it is `false`.
		LinearBar(usize const& layer = 0, bool const manual = false): IGLDrawable(layer, manual) {}

		/// @brief Material to use.
		Material::ObjectMaterial material;

	private:
		/// @brief Underlying vertices to render.
		Vertex vertices[4];

		void draw() override;
	};

	/// @brief Radial progress bar.
	class RadialBar: public IGLDrawable, public Base::Progressbar {
	public:
		/// @brief Whether the progressbar "fans out", or follows a counterclockwise path.
		bool	centered	= false;
		/// @brief Center point offset.
		Vector2 offset		= Vector2(0);

		/// @brief Constructs the progressbar.
		/// @param layer Layer to register the object to. By default, it is layer zero.
		/// @param manual Whether the object is manually rendered. By default, it is `false`.
		RadialBar(usize const& layer = 0, bool const manual = false): IGLDrawable(layer, manual) {
			vertices[0].uv.u = vertices[0].uv.v = 0.5;
		}

		/// @brief Material to use.
		Material::ObjectMaterial material;

	private:
		/// @brief Underlying vertices to render.
		Vertex vertices[RADIAL_BAR_RESOLUTION + 2] = {INITIAL_VERTEX};

		void draw() override;

		void update();
	};

	/// @brief Text label + progressbar combo.
	/// @tparam T Bar type.
	template<BarType T = RadialBar>
	struct LabeledBar {
		/// @brief Bar type.
		using BarType = T;
		/// @brief Progressbar.
		BarType	bar;
		/// @brief Associated label.
		Label	label;
	};
}

#endif // MAKAILIB_GRAPH_RENDERER_BAR_H
