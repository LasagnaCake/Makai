#include "referend.hpp"

using namespace Makai::Graph::Ref;

void Referend::removeReference(IReference& ref)  {
	if (lockState) return;
	if (references.find(&ref) == -1) return;
	triangles.removeRange(ref.triangles.start, ref.triangles.start + ref.triangles.count);
	unbindReference(ref);
}

void Referend::unbindReference(IReference& ref)  {
	if (lockState) return;
	references.eraseLike(&ref);
}

void Referend::transformReferences() {
	for (auto& shape: references)
		shape->transform();
}

void Referend::resetReferenceTransforms() {
	for (auto& shape: references)
		shape->reset();
}

void Referend::clearReferences() {
	if (!references.empty())
		for (auto ref: references) {
			Instance<IReference> iref = ref;
			iref.release();
			delete ref;
		}
	references.clear();
}