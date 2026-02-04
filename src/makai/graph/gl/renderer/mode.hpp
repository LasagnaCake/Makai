#ifndef MAKAILIB_GRAPH_MODE_H
#define MAKAILIB_GRAPH_MODE_H

#include "../../../compat/ctl.hpp"

/// @brief Graphical facilities.
namespace Makai::Graph {
	/// @brief Face culling mode.
	enum class CullMode {
		/// @brief No culling.
		OCM_NONE,
		/// @brief Front face culling.
		OCM_FRONT,
		/// @brief Back face culling.
		OCM_BACK,
		/// @brief Special mode that renders the object two times - first back face, then front face.
		OCM_DUAL,
	};

	/// @brief Face fill mode.
	enum class FillMode {
		OFM_NONE = -1,
		OFM_FILL,
		OFM_LINE,
		OFM_POINT,
	};

	/// @brief Graphical display mode.
	enum class DisplayMode {
		ODM_POINTS,
		ODM_LINE_STRIP,
		ODM_LINE_LOOP,
		ODM_LINES,
		ODM_TRI_STRIP,
		ODM_TRI_FAN,
		ODM_TRIS,
		ODM_LINES_ADJ,
		ODM_LINE_STRIP_ADJ,
		ODM_TRI_ADJ,
		ODM_TRI_STRIP_ADJ,
		ODM_PATCHES
	};

	/// @brief Sets the current face culling mode.
	/// @param mode Mode to use.
	void setCullMode(CullMode const& mode = CullMode::OCM_NONE);
	/// @brief Sets the current face fill mode.
	/// @param mode Mode to use.
	/// @brief Whether mode was `OFM_NONE`.
	bool setFillMode(FillMode const& mode = FillMode::OFM_FILL);

	/// @brief Gets the graphical display mode as the associated OpenGL enum value.
	/// @param mode Mode to convert.
	/// @return Mode as OpenGL enum value.
	uint32 getGLDisplayMode(DisplayMode const& mode = DisplayMode::ODM_TRIS);
}

#endif // MAKAILIB_GRAPH_MODE_H
