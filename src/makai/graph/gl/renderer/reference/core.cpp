#include "core.hpp"
#include "referend.hpp"

using namespace Makai::Graph::Ref;

AReference::~AReference() {
	MAKAILIB_DEBUGLN_ALL("Destroying reference...");
	if (triangles) {
		MAKAILIB_DEBUGLN_ALL("> Clearing triangles...");
		parent.removeReference(*this);
		MAKAILIB_DEBUGLN_ALL("> Triangles cleared!");
	}
}

void AReference::unbind() {
	if (triangles)
		parent.unbindReference(*this);
}

AReference& AReference::transform() {
	if (!triangles || fixed) return *this;
	if (!visible) {
		for (auto& triangle: triangles)
			for (auto& vert: triangle.verts)
				vert.position = 0;
	} else onTransform();
	return *this;
}

AReference& AReference::reset() {
	if (!triangles || fixed) return *this;
	else onReset();
	return *this;
}
