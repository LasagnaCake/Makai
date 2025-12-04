#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_SHAPE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_SHAPE_H

#include "core.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Generic shape reference.
	/// @tparam N Triangle count.
	template<usize N>
	struct AShape: public IReference {
		/// @brief Triangle count.
		constexpr static usize SIZE = N;

		static_assert(N > 0, "Empty shapes are invalid!");

		/// @brief Constructs the reference.
		/// @param triangles Triangles bound to the reference.
		/// @param parent Parent renderable object.
		AShape(
			BoundRange const& triangles,
			Referend& parent
		): IReference(triangles, parent) {}

		/// @brief Destructor.
		virtual ~AShape() {}

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

	/// @brief Type must be a shape reference some kind.
	template<class T>
	concept AShapeType = requires {
		T::SIZE;
	} && Makai::Type::Derived<T, AShape<T::SIZE>> && NotEmpty<T>;
}

#endif