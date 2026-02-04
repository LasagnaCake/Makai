#include "mode.hpp"

#include "../glapiloader.cc"

using namespace Makai;
using namespace Makai; using namespace Makai::Graph;

uint32 convert(DisplayMode const& mode) {
	switch (mode) {
		case DisplayMode::ODM_POINTS:			return GL_POINTS;
		case DisplayMode::ODM_LINE_STRIP:		return GL_LINE_STRIP;
		case DisplayMode::ODM_LINE_LOOP:		return GL_LINE_LOOP;
		case DisplayMode::ODM_LINES:			return GL_LINES;
		case DisplayMode::ODM_TRI_STRIP:		return GL_TRIANGLE_STRIP;
		case DisplayMode::ODM_TRI_FAN:			return GL_TRIANGLE_FAN;
		case DisplayMode::ODM_TRIS:				return GL_TRIANGLES;
		case DisplayMode::ODM_LINES_ADJ:		return GL_LINES_ADJACENCY;
		case DisplayMode::ODM_LINE_STRIP_ADJ:	return GL_LINE_STRIP_ADJACENCY;
		case DisplayMode::ODM_TRI_ADJ:			return GL_TRIANGLES_ADJACENCY;
		case DisplayMode::ODM_TRI_STRIP_ADJ:	return GL_TRIANGLE_STRIP_ADJACENCY;
		case DisplayMode::ODM_PATCHES:			return GL_PATCHES;
		default: throw Error::InvalidValue("Invalid / Unsupported display mode!", CTL_CPP_PRETTY_SOURCE);
	}
}

bool Makai::Graph::setFillMode(FillMode const& mode) {
	switch (mode) {
		case FillMode::OFM_NONE:	return false;
		case FillMode::OFM_FILL:	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	break;
		case FillMode::OFM_POINT:	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);	break;
		case FillMode::OFM_LINE:	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	break;
	}
	return true;
}

void Makai::Graph::setCullMode(CullMode const& mode) {
	switch (mode) {
		case CullMode::OCM_DUAL:
		case CullMode::OCM_NONE:	glDisable(GL_CULL_FACE);						break;
		case CullMode::OCM_FRONT:	glEnable(GL_CULL_FACE); glCullFace(GL_FRONT);	break;
		case CullMode::OCM_BACK:	glEnable(GL_CULL_FACE); glCullFace(GL_BACK);	break;
	}
}

uint32 Makai::Graph::getGLDisplayMode(DisplayMode const& mode) {
	return convert(mode);
}
