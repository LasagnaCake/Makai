#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_H

#include "../../../compat/ctl.hpp"

#include "../vertex.hpp"
#include "../color.hpp"

namespace Makai::Graph {
	/// @brief Renderable object.
	class Renderable;
	/// @brief Renderable object shape reference interface.
	struct IReference;

	/// @brief Renderable object shape reference interface
	struct IReference {
		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		IReference(List<Triangle*> const& triangles, Renderable& parent):
			triangles(triangles),
			parent(parent) {}

		/// @brief Destructor.
		virtual ~IReference() {destroy();}

		/// @brief Bound triangle view type.
		using Triangles = Span<Triangle* const>;

		/// @brief Returns a view to the triangles bound to the reference.
		/// @return View to bound triangles.
		Triangles getBoundTriangles() {
			return Triangles(triangles.begin(), triangles.end());
		}

		/// @brief Resets transformations applied to the bound triangles. Must be implemented.
		/// @return Handle to self.
		virtual Handle<IReference> reset()		= 0;
		/// @brief Applies transformations to the bound triangles. Must be implemented.
		/// @return Handle to self.
		virtual Handle<IReference> transform()	= 0;
		
	protected:
		/// @brief Bound triangles.
		List<Triangle*> const 	triangles;
		/// @brief Parent renderable.
		Renderable& 			parent;
	private:
		void destroy();
		void unbind();
	};

	/// @brief Vertex manipulation function type.
	using VertexFunction = Function<void(Vertex&)>;

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	/// @brief Generic shape reference.
	/// @tparam N Triangle count.
	template<usize N>
	struct ShapeRef: public IReference {
		/// @brief Triangle count.
		constexpr static usize SIZE = N;

		static_assert(N > 0, "Empty shapes are invalid!");

		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		ShapeRef(
			List<Triangle*> const& triangles,
			Renderable& parent
		): IReference(triangles, parent) {}

		/// @brief Destructor.
		virtual ~ShapeRef() {}

		/// @brief Applies an operation to the bound triangles' vertices.
		/// @param f operation to apply.
		virtual void forEachVertex(VertexFunction const& f) {
			for (usize i = 0; i < SIZE; ++i)
				for (usize j = 0; j < 3; ++j)
					f(((Vertex*)triangles[i])[j]);
		}

		/// @brief Resets transformations applied to the bound triangles. Must be implemented.
		/// @return Handle to self.
		virtual Handle<IReference> reset()		= 0;
		/// @brief Applies transformations to the bound triangles. Must be implemented.
		/// @return Handle to self.
		virtual Handle<IReference> transform()	= 0;

		/// @brief Whether transformations should be applied.
		bool fixed		= true;
		/// @brief Whether the reference is visible.
		bool visible	= true;

		/// @brief Transformation.
		Transform3D local;
	};

	#pragma GCC diagnostic pop

	/// @brief Plane reference.
	class PlaneRef: public ShapeRef<2> {
	public:
		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		PlaneRef(
			List<Triangle*> const& triangles,
			Renderable& parent
		);

		/// @brief Destructor.
		virtual ~PlaneRef() {}

		/// @brief Sets the plane's origin.
		/// @param tlPos Top-left corner.
		/// @param trPos Top-right corner.
		/// @param blPos Bottom-left corner.
		/// @param brPos Bottom-right corner.
		/// @return Handle to self.
		Handle<PlaneRef> setOrigin(
			Vector3 const& tlPos = Vector3(-1, +1),
			Vector3 const& trPos = Vector3(+1, +1),
			Vector3 const& blPos = Vector3(-1, -1),
			Vector3 const& brPos = Vector3(+1, -1)
		);

		/// @brief Transforms the plane's origin and normals by a given transform.
		/// @param trans Transform to apply.
		/// @return Handle to self.
		Handle<PlaneRef> setOrigin(Transform3D const& trans);

		/// @brief Sets the plane's UV.
		/// @param tlUV Top-left corner.
		/// @param trUV Top-right corner.
		/// @param blUV Bottom-left corner.
		/// @param brUV Bottom-right corner.
		/// @return Handle to self.
		Handle<PlaneRef> setUV(
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
		Handle<PlaneRef> setColor(
			Vector4 const& tlCol,
			Vector4 const& trCol,
			Vector4 const& blCol,
			Vector4 const& brCol
		);

		/// @brief Sets the plane's color.
		/// @param col Color to set.
		/// @return Handle to self.
		Handle<PlaneRef> setColor(Vector4 const& col = Color::WHITE);

		/// @brief Sets the plane's normal.
		/// @param tln Top-left corner.
		/// @param trn Top-right corner.
		/// @param bln Bottom-left corner.
		/// @param brn Bottom-right corner.
		/// @return Handle to self.
		Handle<PlaneRef> setNormal(
			Vector3 const& tln,
			Vector3 const& trn,
			Vector3 const& bln,
			Vector3 const& brn
		);
		
		/// @brief Sets the plane's normal.
		/// @param n Normal to set.
		/// @return Handle to self.
		Handle<PlaneRef> setNormal(Vector3 const& n);

		/// @brief Resets transformations applied to the bound triangles.
		/// @return Handle to self.
		Handle<IReference> reset() override final;

		/// @brief Applies transformations to the bound triangles.
		/// @return Handle to self.
		Handle<IReference> transform() override final;

		/// @brief Applies an operation to the bound triangles' vertices.
		/// @param f operation to apply.
		void forEachVertex(VertexFunction const& f) override final;

		/// @brief Vertex states pre-transformation.
		Vertex	origin[4];

	protected:
		/// @brief Called when the reference's transforms are applied.
		virtual void onTransform() {};

	private:
		/// @brief Top-left corner's vertex.
		Vertex	*tl		= nullptr;
		/// @brief Top-right corner's first vertex.
		Vertex	*tr1	= nullptr;
		/// @brief Top-right corner's second vertex.
		Vertex	*tr2	= nullptr;
		/// @brief Bottom-left corner's first vertex.
		Vertex	*bl1	= nullptr;
		/// @brief Bottom-left corner's second vertex.
		Vertex	*bl2	= nullptr;
		/// @brief Bottom-right corner's vertex.
		Vertex	*br		= nullptr;
	};

	/// @brief Animated plane reference.
	class AnimatedPlaneRef: public PlaneRef {
	public:
		using PlaneRef::PlaneRef;
		/// @brief Spritesheet frame.
		Vector2 frame;
		/// @brief Spritesheet size.
		Vector2 size = Vector2(1);

	protected:
		/// @brief Called when the reference's transforms are applied.
		void onTransform() override;
	};

	/// @brief Triangle reference.
	class TriangleRef: public ShapeRef<1> {
	public:
		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		TriangleRef(
			List<Triangle*> const& triangles,
			Renderable& parent
		);

		/// @brief Destructor.
		virtual ~TriangleRef() {}

		/// @brief Sets the triangle's origin.
		/// @param aPos First vertex.
		/// @param bPos Second vertex.
		/// @param cPos Third vertex.
		/// @return Handle to self.
		Handle<TriangleRef> setOrigin(
			Vector3 const& aPos = Vector3(+0, +1),
			Vector3 const& bPos = Vector3(-1, -1),
			Vector3 const& cPos = Vector3(+1, -1)
		);

		/// @brief Transforms the triangle's origin and normals by a given transform.
		/// @param trans Transform to apply.
		/// @return Handle to self.
		Handle<TriangleRef> setOrigin(Transform3D const& trans);

		/// @brief Sets the triangle's UV.
		/// @param aUV First vertex.
		/// @param bUV Second vertex.
		/// @param cUV Third vertex.
		/// @return Handle to self.
		Handle<TriangleRef> setUV(
			Vector2 const& aUV,
			Vector2 const& bUV,
			Vector2 const& cUV
		);

		/// @brief Sets the triangle's color.
		/// @param aCol First vertex.
		/// @param bCol Second vertex.
		/// @param cCol Third vertex.
		/// @return Handle to self.
		Handle<TriangleRef> setColor(
			Vector4 const& aCol,
			Vector4 const& bCol,
			Vector4 const& cCol
		);
		/// @brief Sets the triangle's color.
		/// @param col Color to set.
		/// @return Handle to self.
		Handle<TriangleRef> setColor(
			Vector4 const& col = Color::WHITE
		);

		/// @brief Sets the triangle's normal.
		/// @param an First vertex.
		/// @param bn Second vertex.
		/// @param cn Third vertex.
		/// @return Handle to self.
		Handle<TriangleRef> setNormal(
			Vector3 const& an,
			Vector3 const& bn,
			Vector3 const& cn
		);

		/// @brief Sets the triangle's normal.
		/// @param n Normal to set.
		/// @return Handle to self.
		Handle<TriangleRef> setNormal(
			Vector3 const& n
		);

		/// @brief Resets transformations applied to the bound triangles.
		/// @return Handle to self.
		Handle<IReference> reset() override final;

		/// @brief Applies transformations to the bound triangles.
		/// @return Handle to self.
		Handle<IReference> transform() override final;

		/// @brief Applies an operation to the bound triangles' vertices.
		/// @param f operation to apply.
		virtual void forEachVertex(VertexFunction const& f) override final;

		/// @brief Vertex states pre-transformation.
		Vertex origin[3];

	protected:
		/// @brief Called when the reference's transforms are applied.
		virtual void onTransform() {};

	private:
		/// @brief First vertex.
		Vertex	*a	= nullptr;
		/// @brief Second vertex.
		Vertex	*b	= nullptr;
		/// @brief Third vertex.
		Vertex	*c	= nullptr;
	};

	/// @brief Type must not be an empty reference.
	template<class T>
	concept NotEmpty		= Makai::Type::Different<T, IReference>;

	/// @brief Type must be a shape reference some kind.
	template<class T>
	concept ShapeRefType	= requires {
		T::SIZE;
	} && Makai::Type::Derived<T, ShapeRef<T::SIZE>> && NotEmpty<T>;

	/// @brief Type must be a plane reference of some kind.
	template<class T>
	concept PlaneRefType	= Makai::Type::Derived<T, PlaneRef> && NotEmpty<T>;

	/// @brief Type must be a triangle reference of some kind.
	template<class T>
	concept TriangleRefType	= Makai::Type::Derived<T, TriangleRef> && NotEmpty<T>;
}

#endif // MAKAILIB_GRAPH_RENDERER_REFERENCE_H
