#ifndef MAKAILIB_GRAPH_H
#define MAKAILIB_GRAPH_H

namespace Makai::Graph {
	/// @brief Underlying graphical API.
	enum class GraphicalAPI {
		GA_OPENGL/*,
		GA_VULKAN,
		GA_DIRECT3D*/
	};

	/// @brief Returns the underlying graphical API used.
	/// @return Graphical API used.
	GraphicalAPI getUsedAPI();
}

#include "gl/graph.hpp"

namespace MkGraph = Makai::Graph;

#endif // MAKAILIB_GRAPH_H
