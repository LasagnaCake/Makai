#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_TRIANGLE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_TRIANGLE_H

#include "shape.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Triangle reference.
	struct Triangle: AShape<1> {
		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		Triangle(
			BoundRange const& triangles,
			Referend& parent
		);

		/// @brief Destructor.
		virtual ~Triangle() {}

		/// @brief Sets the triangle's origin.
		/// @param aPos First vertex.
		/// @param bPos Second vertex.
		/// @param cPos Third vertex.
		/// @return Handle to self.
		Handle<Triangle> setOrigin(
			Vector3 const& aPos = Vector3(+0, +1),
			Vector3 const& bPos = Vector3(-1, -1),
			Vector3 const& cPos = Vector3(+1, -1)
		);

		/// @brief Transforms the triangle's origin and normals by a given transform.
		/// @param trans Transform to apply.
		/// @return Handle to self.
		Handle<Triangle> setOrigin(Transform3D const& trans);

		/// @brief Sets the triangle's UV.
		/// @param aUV First vertex.
		/// @param bUV Second vertex.
		/// @param cUV Third vertex.
		/// @return Handle to self.
		Handle<Triangle> setUV(
			Vector2 const& aUV,
			Vector2 const& bUV,
			Vector2 const& cUV
		);

		/// @brief Sets the triangle's color.
		/// @param aCol First vertex.
		/// @param bCol Second vertex.
		/// @param cCol Third vertex.
		/// @return Handle to self.
		Handle<Triangle> setColor(
			Vector4 const& aCol,
			Vector4 const& bCol,
			Vector4 const& cCol
		);
		/// @brief Sets the triangle's color.
		/// @param col Color to set.
		/// @return Handle to self.
		Handle<Triangle> setColor(
			Vector4 const& col = Color::WHITE
		);

		/// @brief Sets the triangle's normal.
		/// @param an First vertex.
		/// @param bn Second vertex.
		/// @param cn Third vertex.
		/// @return Handle to self.
		Handle<Triangle> setNormal(
			Vector3 const& an,
			Vector3 const& bn,
			Vector3 const& cn
		);

		/// @brief Sets the triangle's normal.
		/// @param n Normal to set.
		/// @return Handle to self.
		Handle<Triangle> setNormal(
			Vector3 const& n
		);

		/// @brief Resets transformations applied to the bound triangles.
		/// @return Handle to self.
		Handle<AReference> reset() override final;

		/// @brief Applies transformations to the bound triangles.
		/// @return Handle to self.
		Handle<AReference> transform() override final;

		/// @brief Vertex states pre-transformation.
		Vertex origin[3];

	protected:
		/// @brief Called when the reference's transforms are applied.
		virtual void onTransform() {};
	};

	/// @brief Type must be a plane reference of some kind.
	template<class T>
	concept TriangleType	= Makai::Type::Derived<T, Triangle>;
}

#endif