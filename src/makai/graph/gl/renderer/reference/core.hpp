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
		AReference(BoundRange const& triangles, Referend& parent):
			triangles(triangles),
			parent(parent) {}

		/// @brief Destructor.
		virtual ~AReference() {destroy();}

		/// @brief Bound triangle view type.
		using Triangles = Span<Triangle const>;

		/// @brief Returns a view to the triangles bound to the reference.
		/// @return View to bound triangles.
		Triangles getBoundTriangles() const {
			return Triangles(triangles.begin(), triangles.end());
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

		/// @brief Resets transformations applied to the bound triangles. Must be implemented.
		/// @return Handle to self.
		virtual Handle<AReference> reset()		= 0;
		/// @brief Applies transformations to the bound triangles. Must be implemented.
		/// @return Handle to self.
		virtual Handle<AReference> transform()	= 0;
		
	protected:
		friend struct Referend;

		/// @brief Bound triangles.
		BoundRange const 	triangles;
		/// @brief Parent renderable.
		Referend& 			parent;
	private:
		void destroy();
		void unbind();
	};

	/// @brief Type must not be an empty reference.
	template<class T>
	concept NotEmpty		= Makai::Type::Different<T, AReference>;
}

#endif