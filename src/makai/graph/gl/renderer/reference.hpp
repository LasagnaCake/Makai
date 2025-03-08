#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_H

#include "../../../compat/ctl.hpp"

#include "../vertex.hpp"
#include "../color.hpp"

namespace Makai::Graph {
	/// @brief Renderable object.
	struct ReferenceHolder;
	/// @brief Renderable object shape reference interface.
	struct IReference;

	/// @brief Renderable object shape reference interface.
	struct IReference {
		/// @brief Triangle range associated with the reference.
		struct BoundRange {
			/// @brief Mesh containing bound triangles.
			List<Triangle>& mesh;
			/// @brief Start of bound range.
			usize const start;
			/// @brief Amount of triangles bound to the range.
			usize const count;
			/// @brief Returns an iterator to the beginning of the range.
			/// @return Iterator to beginning of range.
			constexpr auto begin() const	{return mesh.begin() + start;	}
			/// @brief Returns an iterator to the end of the range.
			/// @return Iterator to end of range.
			constexpr auto end() const		{return begin() + count;		}
			/// @brief Returns an iterator to the beginning of the range.
			/// @return Iterator to beginning of range.
			constexpr auto begin()			{return mesh.begin() + start;	}
			/// @brief Returns an iterator to the end of the range.
			/// @return Iterator to end of range.
			constexpr auto end()			{return begin() + count;		}
			/// @brief Access operator.
			/// @param index Triangle to get.
			/// @return Reference to triangle.
			constexpr Triangle& operator[](usize const index)		{return mesh[index+start];}
			/// @brief Access operator.
			/// @param index Triangle to get.
			/// @return Reference to triangle.
			constexpr Triangle& operator[](usize const index) const	{return mesh[index+start];}
		};
		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		IReference(BoundRange const& triangles, ReferenceHolder& parent):
			triangles(triangles),
			parent(parent) {}

		/// @brief Destructor.
		virtual ~IReference() {destroy();}

		/// @brief Bound triangle view type.
		using Triangles = Span<Triangle const>;

		/// @brief Returns a view to the triangles bound to the reference.
		/// @return View to bound triangles.
		Triangles getBoundTriangles() const {
			return Triangles(triangles.begin(), triangles.end());
		}

		/// @brief Resets transformations applied to the bound triangles. Must be implemented.
		/// @return Handle to self.
		virtual Handle<IReference> reset()		= 0;
		/// @brief Applies transformations to the bound triangles. Must be implemented.
		/// @return Handle to self.
		virtual Handle<IReference> transform()	= 0;
		
	protected:
		friend class ReferenceHolder;

		/// @brief Bound triangles.
		BoundRange const 	triangles;
		/// @brief Parent renderable.
		ReferenceHolder& 	parent;
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
			BoundRange const& triangles,
			ReferenceHolder& parent
		): IReference(triangles, parent) {}

		/// @brief Destructor.
		virtual ~ShapeRef() {}

		/// @brief Applies an operation to the bound triangles' vertices.
		/// @param f operation to apply.
		virtual void forEachVertex(VertexFunction const& f) {
			for (usize i = 0; i < SIZE; ++i) {
				f(triangles[i].verts[0]);
				f(triangles[i].verts[1]);
				f(triangles[i].verts[2]);
			}
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
			BoundRange const& triangles,
			ReferenceHolder& parent
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
			BoundRange const& triangles,
			ReferenceHolder& parent
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

	struct ReferenceHolder {
		/// @brief Triangle storage container type.
		using TriangleBank = List<Triangle>;

		/// @brief Constructs the reference holder.
		/// @param triangles Reference to triangle bank.
		/// @param lockState Reference to lock state.
		ReferenceHolder(
			TriangleBank& triangles,
			bool& lockState
		): triangles(triangles), lockState(lockState) {}

		/// @brief Destructor.
		virtual ~ReferenceHolder() {
			clearReferences();
		}

		/// @brief Creates a shape reference bound to this object.
		/// @tparam T Reference type.
		/// @return Reference instance.
		template<ShapeRefType T>
		[[nodiscard]]
		Instance<T> createReference() {
			constexpr usize count = T::SIZE;
			if (lockState) throw Error::InvalidAction("Renderable object is locked!", CTL_CPP_PRETTY_SOURCE);
			triangles.expand(count, {});
			// Create shape
			T* shape = new T({triangles, triangles.size()-count, count}, *this);
			// Add to reference list
			references.pushBack(shape);
			// return shape
			return shape;
		}

		/// @brief
		///		Destroys a reference and its associated triangles.
		///		Will only execute if reference is associated with this object.
		/// @tparam T Reference type.
		/// @param ref Reference to remove.
		/// @note If successful, also destroys the reference.
		template <ShapeRefType T>
		void removeReference(Instance<T> const& ref) {
			if (!ref) return;
			if (lockState) return;
			removeReference(*ref);
			ref.destroy();
		}

		/// @brief
		///		Destroys a reference while keeping its associated triangles.
		///		Will only execute if reference is associated with this object.
		/// @tparam T Reference type.
		/// @param ref Reference to remove.
		/// @note If successful, also destroys the reference.
		template <ShapeRefType T>
		void unbindReference(Instance<T> const& ref) {
			if (!ref) return;
			if (lockState) return;
			unbindReference(*ref);
			ref.destroy();
		}

		/// @brief Destroys all bound references.
		void clearReferences();

	protected:
		/// @brief Transforms all bound references.
		void transformReferences();
		/// @brief Resets applied transformation in all bound references.
		void resetReferenceTransforms();

	private:
		void removeReference(IReference& ref);
		void unbindReference(IReference& ref);

		/// @brief References bound to this object.
		List<IReference*>	references;

		friend class IReference;

		/// @brief Triangle bank to store triangles into. 
		TriangleBank&	triangles;
		/// @brief Triangle bank lock state.
		bool&			lockState;
	};
}

#endif // MAKAILIB_GRAPH_RENDERER_REFERENCE_H
