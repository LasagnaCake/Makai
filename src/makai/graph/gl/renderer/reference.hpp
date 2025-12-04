#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_H_COMPAT
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_H_COMPAT

#include "reference/reference.hpp"

#define MAKAI_GRR_DEPRECATED [[deprecated("Please use the equivalent type on the `Ref` namespace!")]]

namespace Makai::Graph {
	using IReference		MAKAI_GRR_DEPRECATED	= Ref::IReference;
	using ReferenceHolder	MAKAI_GRR_DEPRECATED	= Ref::Referend;
	template <usize N>
	using ShapeRef			MAKAI_GRR_DEPRECATED	= Ref::AShape<N>;
	using PlaneRef			MAKAI_GRR_DEPRECATED	= Ref::Plane;
	using AnimatedPlaneRef	MAKAI_GRR_DEPRECATED	= Ref::SpritePlane;
	using TriangleRef		MAKAI_GRR_DEPRECATED	= Ref::Triangle;
}

#endif // MAKAILIB_GRAPH_RENDERER_REFERENCE_H_COMPAT
