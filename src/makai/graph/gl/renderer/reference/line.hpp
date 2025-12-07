#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_LINE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_LINE_H

#include "shape.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Line reference.
	struct Line: AShape<2> {
		/// @brief Line tip.
		struct Tip {
			/// @brief Tip position.
			Vector3	position;
			/// @brief Tip length.
			float	width;
			/// @brief Tip angle.
			float	angle;
		};

		/// @brief Line start.
		Tip from;
		/// @brief Line end.
		Tip to;

		/// @brief Sets the line's color.
		/// @param col Color to set.
		/// @return Reference to self.
		Line& setColor(Vector4 const& col = Color::WHITE);

	private:
		/// @brief Applies transformations to the bound triangles.
		/// @return Reference to self.
		void onTransform() override final;
		/// @brief Resets transformations applied to the bound triangles.
		/// @return Reference to self.
		void onReset() override final;

		void getBaseShape(As<Vertex[4]>& vertices);
	};
}

#endif