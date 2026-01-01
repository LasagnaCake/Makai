#include "triangle.hpp"
#include "srptransform.cc"

using namespace Makai::Graph::Ref;

Triangle::Triangle(
	AReference::BoundRange const& tris,
	Referend& parent
): AShape<1>(tris, parent) {
	// Setup trigon
	setOrigin(
		Vector3(-0.0, +1.0, 0.0),
		Vector3(-1.0, -1.0, 0.0),
		Vector3(+1.0, -1.0, 0.0)
	);
	setUV(
		Vector2(+0.5, +1.0),
		Vector2(+0.0, +0.0),
		Vector2(+1.0, +0.0)
	);
	setColor();
	setNormal(
		Vector3(+0.0, +0.0, -1.0)
	);
}

/// Sets the triangle's origin.
Triangle& Triangle::setOrigin(
	Vector3 const& aPos,
	Vector3 const& bPos,
	Vector3 const& cPos
) {
	origin[0].position = (aPos);
	origin[1].position = (bPos);
	origin[2].position = (cPos);
	return *this;
}

/// Transforms the triangle's origin and normals by a given transform.
Triangle& Triangle::setOrigin(Transform3D const& trans) {
	Matrix4x4 tmat(trans);
	Matrix3x3 nmat(tmat.transposed().inverted().truncated(3, 3));
	srpTransform(origin[0], tmat, nmat);
	srpTransform(origin[1], tmat, nmat);
	srpTransform(origin[2], tmat, nmat);
	return *this;
}

Triangle& Triangle::setUV(
	Vector2 const& aUV,
	Vector2 const& bUV,
	Vector2 const& cUV
) {
	origin[0].uv = (aUV);
	origin[1].uv = (bUV);
	origin[2].uv = (cUV);
	return *this;
}

Triangle& Triangle::setColor(
	Vector4 const& aCol,
	Vector4 const& bCol,
	Vector4 const& cCol
) {
	origin[0].color = (aCol);
	origin[1].color = (bCol);
	origin[2].color = (cCol);
	return *this;
}

Triangle& Triangle::setColor(Vector4 const& col) {
	setColor(col, col, col);
	return *this;
}

Triangle& Triangle::setNormal(
	Vector3 const& an,
	Vector3 const& bn,
	Vector3 const& cn
) {
	origin[0].normal = (an);
	origin[1].normal = (bn);
	origin[2].normal = (cn);
	return *this;
}

Triangle& Triangle::setNormal(Vector3 const& n) {
	setNormal(n, n, n);
	return *this;
}

/// Sets the triangle to its original state (last state set with setPosition).
void Triangle::onReset() {
	if (fixed) return;
	As<Vertex&>
		a	= (triangles[0].verts[0]),
		b	= (triangles[0].verts[1]),
		c	= (triangles[0].verts[0])
	;
	a = origin[0];
	b = origin[1];
	c = origin[2];
}

void Triangle::onTransform() {
	if (fixed) return;
	As<Vertex&>
		a	= (triangles[0].verts[0]),
		b	= (triangles[0].verts[1]),
		c	= (triangles[0].verts[0])
	;
	// Calculate transformed vertices
	Vertex tri[3] = {origin[0], origin[1], origin[2]};
	if (visible) {
		Matrix4x4 tmat(matrix());
		Matrix3x3 nmat(tmat.transposed().inverted().truncated(3, 3));
		for (Graph::Vertex& vert: tri) {
			vert.position	= tmat * Vector4(vert.position, 1);
			vert.normal		= nmat * vert.normal;
		}
	} else for (auto& vert: tri)
		vert.position = 0;
	// Apply transformation
	a	= tri[0];
	b	= tri[1];
	c	= tri[2];
}