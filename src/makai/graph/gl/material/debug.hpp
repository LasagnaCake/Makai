#ifndef MAKAILIB_GRAPH_MATERIAL_DEBUG_H
#define MAKAILIB_GRAPH_MATERIAL_DEBUG_H

/// @brief Shader materials.
namespace Makai::Graph::Material {
	/// @brief Framebuffer debug view mode.
	enum class BufferDebugView: unsigned int {
		BDV_NONE = 0,
		BDV_DEPTH,
	};

	/// @brief Object debug view mode.
	enum class ObjectDebugView: unsigned int {
		ODV_NONE = 0,
		ODV_NORMAL,
	};
}

#endif // MAKAILIB_GRAPH_MATERIAL_DEBUG_H
