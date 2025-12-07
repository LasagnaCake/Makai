#include "line.hpp"

using namespace Makai::Graph::Ref;

void Line::onTransform() {
	if (fixed) return;
	As<Vertex[4]> plane;
	getBaseShape(plane);
	Makai::Math::Mat4 tmat(local);
	Makai::Math::Mat3 nmat(tmat.transposed().inverted().truncated(3, 3));
	for (auto& vert: plane) {
		vert.normal = Makai::Vector3::FRONT();
		if (visible) {
			vert.position	= tmat * Makai::Math::Vector4(vert.position, 1);
			vert.normal		= nmat * vert.normal;
		}
		else vert.position = 0; 
	}
	triangles[0].verts[0] = plane[0];
	triangles[0].verts[1] = plane[1];
	triangles[0].verts[2] = plane[2];
	triangles[1].verts[0] = plane[1];
	triangles[1].verts[1] = plane[2];
	triangles[1].verts[2] = plane[3];
}

void Line::onReset() {
	if (fixed) return;
	for (auto& triangle: triangles)
		for (auto& vert: triangle.verts)
			vert.position = 0; 
}

void Line::getBaseShape(As<Vertex[4]>& vertices) {
	Vector3 const normal	= from.position.normalTo(to.position);
	float const angle		= from.position.xz().angleTo(to.position.xz());
	Vector3 const lhsFrom	= Makai::Math::rotateV3(Vector3::LEFT(),	{0, angle, from.angle});
	Vector3 const rhsFrom	= Makai::Math::rotateV3(Vector3::RIGHT(),	{0, angle, from.angle});
	Vector3 const lhsTo		= Makai::Math::rotateV3(Vector3::LEFT(),	{0, angle, to.angle});
	Vector3 const rhsTo		= Makai::Math::rotateV3(Vector3::RIGHT(),	{0, angle, to.angle});
	vertices[0].position = lhsFrom * from.width + from.position;
	vertices[1].position = rhsFrom * from.width + from.position;
	vertices[2].position = lhsTo * to.width + to.position;
	vertices[3].position = rhsTo * to.width + to.position;
}

Line& Line::setColor(Makai::Vector4 const& col) {
	for (auto& triangle: triangles)
		for (auto& vert: triangle.verts)
			vert.color = col;
	return *this;
}