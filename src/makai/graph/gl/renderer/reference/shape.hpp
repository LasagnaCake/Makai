#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_SHAPE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_SHAPE_H

#include "core.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Generic shape reference.
	/// @tparam N Triangle count.
	template<usize N>
	struct AShape: public AReference {
		/// @brief Triangle count.
		constexpr static usize SIZE = N;

		static_assert(N > 0, "Empty shapes are invalid!");

		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		AShape(
			BoundRange const& triangles,
			Referend& parent
		): AReference(triangles, parent) {}

		/// @brief Destructor.
		virtual ~AShape() {}

		/// @brief Resets transformations applied to the bound triangles. Must be implemented.
		virtual void onReset()		= 0;
		/// @brief Applies transformations to the bound triangles. Must be implemented.
		virtual void onTransform()	= 0;

		/// @brief Transformation.
		Transform3D local;

	protected:
		/// @brief Applies the local transformation matrix to all the vertices.
		void applyTransform() {
			Matrix4x4 tmat(local);
			Matrix3x3 nmat(tmat.transposed().inverted().truncated(3, 3));
			for (auto& triangle: triangles)
				for (auto& vert: triangle.verts) {
					vert.position	= tmat * Vector4(vert.position, 1);
					vert.normal		= nmat * vert.normal;
				}
		}
	};

	/// @brief Type must be a shape reference some kind.
	template<class T>
	concept AShapeType = requires {
		T::SIZE;
	} && Makai::Type::Derived<T, AShape<T::SIZE>> && NotEmpty<T>;
}

#endif