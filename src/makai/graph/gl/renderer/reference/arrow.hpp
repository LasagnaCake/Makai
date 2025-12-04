#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_ARROW_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_ARROW_H

#include "shape.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Arrow reference.
	struct Arrow: AShape<4> {
		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		Arrow(
			BoundRange const&	triangles,
			Referend&			parent
		): AShape(triangles, parent) {
			setBaseShape();
		}
		
		/// @brief Arrow tip.
		struct Tip {
			/// @brief Tip width.
			float width		= 1;
			/// @brief Tip length.
			float length	= 1;
		};
		
		/// @brief Arrow body.
		struct Body {
			/// @brief Body start width.
			float begin		= 1;
			/// @brief Body end width.
			float end		= 1;
			/// @brief Body length.
			float length	= 1;
		};

		/// @brief Arrow alignment along local origin.
		/// @note 0 = Left-to-Right, 1 = Right-to-Left. 0.5 = Centered.
		float align	= 0;

		/// @brief Arrow head shape.
		Tip		head	= {2, 1};
		/// @brief Arrow body shape.
		Body	body	= {1, 1, 1};
		/// @brief Arrow tail shape.
		Tip		tail	= {2, 1};
		
		/// @brief Resets transformations applied to the bound triangles.
		/// @return Handle to self.
		Handle<AReference> reset() override final;

		/// @brief Applies transformations to the bound triangles.
		/// @return Handle to self.
		Handle<AReference> transform() override final;

		/// @brief Sets the arrow's color.
		/// @param col Color to set.
		/// @return Handle to self.
		Handle<Arrow> setColor(Vector4 const& col = Color::WHITE);

	private:
		void setBaseShape();
	};
}

#endif