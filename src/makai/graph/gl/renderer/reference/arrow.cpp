#include "arrow.hpp"

using namespace Makai::Graph::Ref;

Makai::Handle<AReference> Arrow::reset() {
	for (auto& triangle: triangles)
		for (auto& vert: triangle.verts)
			vert.position = 0; 
	return this;
}

Makai::Handle<AReference> Arrow::transform() {
	if (!fixed) return this;
	setBaseShape();
	Makai::Math::Mat4 tmat(local);
	Makai::Math::Mat3 nmat(tmat.transposed().inverted().truncated(3, 3));
	for (auto& triangle: triangles)
		for (auto& vert: triangle.verts) {
			vert.normal = Makai::Vector3::FRONT();
			if (visible) {
				vert.position	= tmat * Makai::Math::Vector4(vert.position, 1);
				vert.normal		= nmat * vert.normal;
			}
			else vert.position = 0; 
		}
	return this;
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

Makai::Handle<Arrow> Arrow::setColor(Makai::Vector4 const& col) {
	for (auto& triangle: triangles)
		for (auto& vert: triangle.verts)
			vert.color = col;
	return this;
}