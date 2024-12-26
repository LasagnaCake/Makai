#ifndef MAKAILIB_GRAPH_RENDERER_DUMMY_H
#define MAKAILIB_GRAPH_RENDERER_DUMMY_H

#include "drawable.hpp"

namespace Makai::Graph {
	class Dummy: public IDrawable {
	public:
		/// @brief Constructs the dummy.
		/// @param layers Layers to insert the dummy in.
		Dummy(Arguments<usize> const& layers) {
			for (usize const l : layers)
				addToRenderLayer(l);
		}

		/// @brief Does nothing.
		void draw() override final {};
	};
}

#endif // MAKAILIB_GRAPH_RENDERER_DUMMY_H
