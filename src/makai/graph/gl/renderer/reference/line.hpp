#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_LINE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_LINE_H

#include "shape.hpp"

namespace Makai::Graph::Ref {
	struct Line: AShape<2> {
		struct Tip {
			Vector3	position;
			float	width;
			float	angle;
		};

		Tip from;
		Tip to;

		Handle<IReference> transform() override final;
		Handle<IReference> reset() override final;

	private:
		void getBaseShape(As<Vertex[4]>& vertices);
	};
}

#endif