#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_CORE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_CORE_H

#include "../../../../compat/ctl.hpp"

#include "../../vertex.hpp"
#include "../../../color/color.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Reference holder.
	struct Referend;
	/// @brief Reference holder shape reference interface.
	struct AReference;

	/// @brief GReference holder shape reference interface.
	struct AReference {
		/// @brief Triangle range associated with the reference.
		struct BoundRange {
			using MeshReference = Reference<List<Triangle>>;

			constexpr BoundRange(MeshReference const& mesh, usize const start, usize const count):
				count(count),
				start(start),
				mesh(mesh) {}
			/// @brief Amount of triangles bound to the range.
			usize const	count;
			/// @brief Returns an iterator to the beginning of the range.
			/// @return Iterator to beginning of range.
			constexpr auto begin() const	{return mesh->begin() + start;	}
			/// @brief Returns an iterator to the end of the range.
			/// @return Iterator to end of range.
			constexpr auto end() const		{return begin() + count;		}
			/// @brief Returns an iterator to the beginning of the range.
			/// @return Iterator to beginning of range.
			constexpr auto begin()			{return mesh->begin() + start;	}
			/// @brief Returns an iterator to the end of the range.
			/// @return Iterator to end of range.
			constexpr auto end()			{return begin() + count;		}
			/// @brief Access operator.
			/// @param index Triangle to get.
			/// @return Reference to triangle.
			constexpr Triangle& operator[](usize const index)		{return (*mesh)[index+start];}
			/// @brief Access operator.
			/// @param index Triangle to get.
			/// @return Reference to triangle.
			constexpr Triangle& operator[](usize const index) const	{return (*mesh)[index+start];}

			constexpr operator bool() const {return mesh.exists();}

		private:
			/// @brief Start of bound range.
			usize start;

			friend struct Referend;
			/// @brief Mesh containing bound triangles.
			Reference<List<Triangle>> mesh;
		};

		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		AReference(BoundRange const& triangles, Referend& parent):
			triangles(triangles),
			parent(parent) {}

		/// @brief Destructor.
		virtual ~AReference();

		/// @brief Bound triangle view type.
		using Triangles = Span<Triangle const>;

		/// @brief Returns a view to the triangles bound to the reference.
		/// @return View to bound triangles.
		Triangles getBoundTriangles() const {
			if (triangles)
				return Triangles(triangles.begin(), triangles.end());
			else return Triangles();
		}

		/// @brief Applies a function on every vertex in the reference.
		/// @tparam TFunction Function type.
		/// @param f Function to apply.
		template<Type::Functional<void(Vertex&)> TFunction>
		constexpr void forEachVertex(TFunction const& f) {
			for (auto& triangle: triangles)
				for (auto& vert: triangle.verts)
					f(vert);
		}

		/// @brief Resets transformations applied to the bound triangles.
		/// @return Reference to self.
		AReference& reset();
		/// @brief Applies transformations to the bound triangles.
		/// @return Reference to self.
		AReference& transform();

		/// @brief Unbinds the reference from its parent.
		void unbind();

		/// @brief Whether transforming/resetting should be forbidden.
		bool fixed		= false;
		/// @brief Whether the reference is visible.
		bool visible	= true;
		
	protected:
		/// @brief Called when transformations are resetted. Must be implemented.
		/// @return Reference to self.
		virtual void onReset()		= 0;
		/// @brief Called when transformations are requested. Must be implemented.
		/// @return Reference to self.
		virtual void onTransform()	= 0;

		friend struct Referend;

		/// @brief Bound triangles.
		BoundRange	triangles;
		/// @brief Parent renderable.
		Referend&	parent;
	private:
	};

	/// @brief Type must not be an empty reference.
	template<class T>
	concept NotEmpty		= Makai::Type::Different<T, AReference>;
}

#endif