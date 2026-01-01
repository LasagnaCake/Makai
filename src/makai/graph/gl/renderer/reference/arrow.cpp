#include "arrow.hpp"

using namespace Makai::Graph::Ref;

void Arrow::onReset() {
	if (fixed) return;
	for (auto& triangle: triangles)
		for (auto& vert: triangle.verts)
			vert.position = 0; 
}

void Arrow::onTransform() {
	if (fixed) return;
	setBaseShape();
	applyTransform();
}

void Arrow::setBaseShape() {
	float const mainPart = head.length + body.length;
	float const size = head.length + body.length + tail.length;
	float const offset = size * align;
	triangles[0].verts[2].position = Makai::Vector2(-head.length + offset, -head.width / 2);
	triangles[0].verts[1].position = 0;
	triangles[0].verts[0].position = Makai::Vector2(-head.length + offset, +head.width / 2);
	triangles[1].verts[2].position = Makai::Vector2(-mainPart + offset, -body.end / 2);
	triangles[1].verts[1].position = Makai::Vector2(-head.length, +body.begin / 2);
	triangles[1].verts[0].position = Makai::Vector2(-mainPart + offset, +body.end / 2);
	triangles[2].verts[2].position = Makai::Vector2(-head.length, +body.begin / 2);
	triangles[2].verts[1].position = Makai::Vector2(-head.length, -body.begin / 2);
	triangles[2].verts[0].position = Makai::Vector2(-mainPart + offset, -body.end / 2);
	triangles[3].verts[2].position = Makai::Vector2(-mainPart + offset, -tail.width / 2);
	triangles[3].verts[1].position = Makai::Vector2(-size + offset, 0);
	triangles[3].verts[0].position = Makai::Vector2(-mainPart + offset, +tail.width / 2);
	
}

Arrow& Arrow::setColor(Makai::Vector4 const& col) {
	for (auto& triangle: triangles)
		for (auto& vert: triangle.verts)
			vert.color = col;
	return *this;
}