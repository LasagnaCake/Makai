#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_PLANE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_PLANE_H

#include "shape.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Plane reference.
	struct Plane: AShape<2> {
	public:
		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		Plane(
			BoundRange const& triangles,
			Referend& parent
		);

		/// @brief Destructor.
		virtual ~Plane() {}

		/// @brief Sets the plane's origin.
		/// @param tlPos Top-left corner.
		/// @param trPos Top-right corner.
		/// @param blPos Bottom-left corner.
		/// @param brPos Bottom-right corner.
		/// @return Handle to self.
		Handle<Plane> setOrigin(
			Vector3 const& tlPos = Vector3(-1, +1),
			Vector3 const& trPos = Vector3(+1, +1),
			Vector3 const& blPos = Vector3(-1, -1),
			Vector3 const& brPos = Vector3(+1, -1)
		);

		/// @brief Transforms the plane's origin and normals by a given transform.
		/// @param trans Transform to apply.
		/// @return Handle to self.
		Handle<Plane> setOrigin(Transform3D const& trans);

		/// @brief Sets the plane's UV.
		/// @param tlUV Top-left corner.
		/// @param trUV Top-right corner.
		/// @param blUV Bottom-left corner.
		/// @param brUV Bottom-right corner.
		/// @return Handle to self.
		Handle<Plane> setUV(
			Vector2 const& tlUV,
			Vector2 const& trUV,
			Vector2 const& blUV,
			Vector2 const& brUV
		);

		/// @brief Sets the plane's color.
		/// @param tlCol Top-left corner.
		/// @param trCol Top-right corner.
		/// @param blCol Bottom-left corner.
		/// @param brCol Bottom-right corner.
		/// @return Handle to self.
		Handle<Plane> setColor(
			Vector4 const& tlCol,
			Vector4 const& trCol,
			Vector4 const& blCol,
			Vector4 const& brCol
		);

		/// @brief Sets the plane's color.
		/// @param col Color to set.
		/// @return Handle to self.
		Handle<Plane> setColor(Vector4 const& col = Color::WHITE);

		/// @brief Sets the plane's normal.
		/// @param tln Top-left corner.
		/// @param trn Top-right corner.
		/// @param bln Bottom-left corner.
		/// @param brn Bottom-right corner.
		/// @return Handle to self.
		Handle<Plane> setNormal(
			Vector3 const& tln,
			Vector3 const& trn,
			Vector3 const& bln,
			Vector3 const& brn
		);
		
		/// @brief Sets the plane's normal.
		/// @param n Normal to set.
		/// @return Handle to self.
		Handle<Plane> setNormal(Vector3 const& n);

		/// @brief Resets transformations applied to the bound triangles.
		/// @return Handle to self.
		Handle<IReference> reset() override final;

		/// @brief Applies transformations to the bound triangles.
		/// @return Handle to self.
		Handle<IReference> transform() override final;

		/// @brief Vertex states pre-transformation.
		Vertex	origin[4];

	protected:
		/// @brief Called when the reference's transforms are applied.
		virtual void onTransform() {};
	};
	
	/// @brief Spritesheet plane reference.
	struct SpritePlane: Plane {
	public:
		using Plane::Plane;
		/// @brief Spritesheet frame.
		Vector2 frame;
		/// @brief Spritesheet size.
		Vector2 size = Vector2(1);

	protected:
		/// @brief Called when the reference's transforms are applied.
		void onTransform() override;
	};
	
	/// @brief Type must be a plane reference of some kind.
	template<class T>
	concept PlaneType	= Makai::Type::Derived<T, Plane>;
}

#endif