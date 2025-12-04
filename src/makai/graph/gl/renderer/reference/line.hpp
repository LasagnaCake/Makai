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

		/// @brief Applies transformations to the bound triangles.
		/// @return Handle to self.
		Handle<AReference> transform() override final;
		/// @brief Resets transformations applied to the bound triangles.
		/// @return Handle to self.
		Handle<AReference> reset() override final;

	private:
		void getBaseShape(As<Vertex[4]>& vertices);
	};
}

#endif