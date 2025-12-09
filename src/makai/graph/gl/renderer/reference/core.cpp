#include "core.hpp"
#include "referend.hpp"

using namespace Makai::Graph::Ref;

AReference::~AReference() {
	DEBUGLN("Clearing reference...");
	if (triangles)
		parent.removeReference(*this);
	DEBUGLN("Reference cleared!");
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