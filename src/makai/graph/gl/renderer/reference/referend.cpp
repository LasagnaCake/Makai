#include "referend.hpp"

using namespace Makai::Graph::Ref;

Referend::~Referend() {
	clear();
}

void Referend::removeReference(AReference& ref)  {
	if (lockState) return;
	if (references.find(&ref) == -1) return;
	if (!ref.triangles) return;
	for (auto& triangle: ref.triangles)
		triangle = {};
	//triangles.eraseRange(ref.triangles.start, ref.triangles.start + ref.triangles.count);
	unbindReference(ref);
}

void Referend::unbindReference(AReference& ref)  {
	if (lockState) return;
	ref.triangles.mesh = nullptr;
	references.eraseLike(&ref);
}

void Referend::transformAll() {
	for (auto& shape: references)
		shape->transform();
}

void Referend::resetAll() {
	for (auto& shape: references)
		shape->reset();
}

void Referend::clear() {
	if (!references.empty())
		for (auto ref: references)
			ref->triangles.mesh = nullptr;
	references.clear();
}