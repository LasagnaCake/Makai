
#include "plane.hpp"
#include "srptransform.cc"

using namespace Makai::Graph::Ref;

Plane::Plane(
	BoundRange const& tris,
	Referend& parent
): AShape<2>(tris, parent) {
	// Setup plane
	this->setOrigin(
		Vector3(-1.0, +1.0, 0.0),
		Vector3(+1.0, +1.0, 0.0),
		Vector3(-1.0, -1.0, 0.0),
		Vector3(+1.0, -1.0, 0.0)
	);
	this->setUV(
		Vector2(+0.0, +1.0),
		Vector2(+1.0, +1.0),
		Vector2(+0.0, +0.0),
		Vector2(+1.0, +0.0)
	);
	this->setColor();
	this->setNormal(
		Vector3(+0.0, +0.0, -1.0)
	);
}

/// Sets the plane's origin.
Makai::Handle<Plane> Plane::setOrigin(
	Vector3 const& tlPos,
	Vector3 const& trPos,
	Vector3 const& blPos,
	Vector3 const& brPos
) {
	origin[0].position = (tlPos);
	origin[1].position = (trPos);
	origin[2].position = (blPos);
	origin[3].position = (brPos);
	return this;
}

/// Transforms the plane's origin and normals by a given transform.
Makai::Handle<Plane> Plane::setOrigin(Transform3D const& trans) {
	Matrix4x4 tmat(trans);
	Matrix3x3 nmat(tmat.transposed().inverted().truncated(3, 3));
	srpTransform(origin[0],	tmat, nmat);
	srpTransform(origin[1],	tmat, nmat);
	srpTransform(origin[2],	tmat, nmat);
	srpTransform(origin[3],	tmat, nmat);
	return this;
}

Makai::Handle<Plane> Plane::setUV(
		Vector2 const& tlUV,
		Vector2 const& trUV,
		Vector2 const& blUV,
		Vector2 const& brUV
	) {
	origin[0].uv = (tlUV);
	origin[1].uv = (trUV);
	origin[2].uv = (blUV);
	origin[3].uv = (brUV);
	return this;
}

Makai::Handle<Plane> Plane::setColor(
		Vector4 const& tlCol,
		Vector4 const& trCol,
		Vector4 const& blCol,
		Vector4 const& brCol
	) {
	origin[0].color = (tlCol);
	origin[1].color = (trCol);
	origin[2].color = (blCol);
	origin[3].color = (brCol);
	return this;
}

Makai::Handle<Plane> Plane::setColor(Vector4 const& col) {
	setColor(col, col, col, col);
	return this;
}

Makai::Handle<Plane> Plane::setNormal(
		Vector3 const& tln,
		Vector3 const& trn,
		Vector3 const& bln,
		Vector3 const& brn
	) {
	origin[0].normal = (tln);
	origin[1].normal = (trn);
	origin[2].normal = (bln);
	origin[3].normal = (brn);
	return this;
}

Makai::Handle<Plane> Plane::setNormal(
		Vector3 const& n
	) {
	setNormal(n, n, n, n);
	return this;
}

/// Sets the plane to its original state (last state set with setPosition).
Makai::Handle<AReference> Plane::reset() {
	As<Vertex&>
		tl	= (triangles[0].verts[0]),
		tr1	= (triangles[0].verts[1]),
		tr2	= (triangles[1].verts[0]),
		bl1	= (triangles[0].verts[2]),
		bl2	= (triangles[1].verts[2]),
		br	= (triangles[1].verts[1])
	;
	// Set origin
	tl			= origin[0];
	tr1	= tr2	= origin[1];
	bl1	= bl2	= origin[2];
	br			= origin[3];
	return this;
}

Makai::Handle<AReference> Plane::transform() {
	As<Vertex&>
		tl	= (triangles[0].verts[0]),
		tr1	= (triangles[0].verts[1]),
		tr2	= (triangles[1].verts[0]),
		bl1	= (triangles[0].verts[2]),
		bl2	= (triangles[1].verts[2]),
		br	= (triangles[1].verts[1])
	;
	onTransform();
	if (!fixed) return this;
	// Calculate transformed vertices
	Vertex plane[4] = {origin[0], origin[1], origin[2], origin[3]};
	if (visible) {
		Matrix4x4 tmat(local);
		Matrix3x3 nmat(tmat.transposed().inverted().truncated(3, 3));
		for (Graph::Vertex& vert: plane) {
			vert.position	= tmat * Vector4(vert.position, 1);
			vert.normal		= nmat * vert.normal;
		}
	} else for (auto& vert: plane)
		vert.position = 0;
	// Apply transformation
	tl			= plane[0];
	tr1	= tr2	= plane[1];
	bl1	= bl2	= plane[2];
	br			= plane[3];
	return this;
}

void SpritePlane::onTransform() {
	if (size.x == 0 || size.y == 0)
		setUV(0, 0, 0, 0);
	else setUV(
		(frame) / size,
		(frame + Vector2(1, 0)) / size,
		(frame + Vector2(0, 1)) / size,
		(frame + Vector2(1)) / size
	);
}