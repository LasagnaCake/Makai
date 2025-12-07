
#include "plane.hpp"
#include "srptransform.cc"

using namespace Makai::Graph::Ref;

Plane::Plane(
	BoundRange const& tris,
	Referend& parent
): AShape<2>(tris, parent) {
	// Setup plane
	setOrigin(
		Vector3(-1.0, +1.0, 0.0),
		Vector3(+1.0, +1.0, 0.0),
		Vector3(-1.0, -1.0, 0.0),
		Vector3(+1.0, -1.0, 0.0)
	);
	setUV(
		Vector2(+0.0, +1.0),
		Vector2(+1.0, +1.0),
		Vector2(+0.0, +0.0),
		Vector2(+1.0, +0.0)
	);
	setColor();
	setNormal(
		Vector3(+0.0, +0.0, -1.0)
	);
}

/// Sets the plane's origin.
Plane& Plane::setOrigin(
	Vector3 const& tlPos,
	Vector3 const& trPos,
	Vector3 const& blPos,
	Vector3 const& brPos
) {
	origin[0].position = (tlPos);
	origin[1].position = (trPos);
	origin[2].position = (blPos);
	origin[3].position = (brPos);
	return *this;
}

/// Transforms the plane's origin and normals by a given transform.
Plane& Plane::setOrigin(Transform3D const& trans) {
	Matrix4x4 tmat(trans);
	Matrix3x3 nmat(tmat.transposed().inverted().truncated(3, 3));
	srpTransform(origin[0],	tmat, nmat);
	srpTransform(origin[1],	tmat, nmat);
	srpTransform(origin[2],	tmat, nmat);
	srpTransform(origin[3],	tmat, nmat);
	return *this;
}

Plane& Plane::setUV(
	Vector2 const& tlUV,
	Vector2 const& trUV,
	Vector2 const& blUV,
	Vector2 const& brUV
) {
	origin[0].uv = (tlUV);
	origin[1].uv = (trUV);
	origin[2].uv = (blUV);
	origin[3].uv = (brUV);
	return *this;
}

Plane& Plane::setColor(
	Vector4 const& tlCol,
	Vector4 const& trCol,
	Vector4 const& blCol,
	Vector4 const& brCol
) {
	origin[0].color = (tlCol);
	origin[1].color = (trCol);
	origin[2].color = (blCol);
	origin[3].color = (brCol);
	return *this;
}

Plane& Plane::setColor(Vector4 const& col) {
	setColor(col, col, col, col);
	return *this;
}

Plane& Plane::setNormal(
		Vector3 const& tln,
		Vector3 const& trn,
		Vector3 const& bln,
		Vector3 const& brn
	) {
	origin[0].normal = (tln);
	origin[1].normal = (trn);
	origin[2].normal = (bln);
	origin[3].normal = (brn);
	return *this;
}

Plane& Plane::setNormal(Vector3 const& n) {
	setNormal(n, n, n, n);
	return *this;
}

/// Sets the plane to its original state (last state set with setPosition).
void Plane::onReset() {
	if (fixed) return;
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
}

void Plane::onTransform() {
	if (fixed) return;
	As<Vertex&>
		tl	= (triangles[0].verts[0]),
		tr1	= (triangles[0].verts[1]),
		tr2	= (triangles[1].verts[0]),
		bl1	= (triangles[0].verts[2]),
		bl2	= (triangles[1].verts[2]),
		br	= (triangles[1].verts[1])
	;
	// Calculate transformed vertices
	Vertex plane[4] = {origin[0], origin[1], origin[2], origin[3]};
	Matrix4x4 tmat(local);
	Matrix3x3 nmat(tmat.transposed().inverted().truncated(3, 3));
	for (Graph::Vertex& vert: plane) {
		vert.position	= tmat * Vector4(vert.position, 1);
		vert.normal		= nmat * vert.normal;
	}
	// Apply transformation
	tl			= plane[0];
	tr1	= tr2	= plane[1];
	bl1	= bl2	= plane[2];
	br			= plane[3];
}

void FractionTilePlane::onTransform() {
	if (fixed) return;
	Plane::onTransform();
	if (size.x == 0 || size.y == 0)
		setUV(0, 0, 0, 0);
	else {
		auto const sz = size;
		Vector2 const f = tile / sz;
		setUV(
			f,
			f + (Vector2::RIGHT()	/ sz),
			f + (Vector2::UP()		/ sz),
			f + (Vector2::ONE()		/ sz)
		);
	}
}

void TilePlane::onTransform() {
	if (fixed) return;
	Plane::onTransform();
	if (size.x == 0 || size.y == 0)
		setUV(0, 0, 0, 0);
	else {
		auto const sz = size.toVector2();
		Vector2 const f = tile.toVector2() / sz;
		setUV(
			f,
			f + (Vector2::RIGHT()	/ sz),
			f + (Vector2::UP()		/ sz),
			f + (Vector2::ONE()		/ sz)
		);
	}
}

void AnimationPlane::onTransform() {
	if (fixed) return;
	Plane::onTransform();
	if (size.x == 0 || size.y == 0)
		setUV(0, 0, 0, 0);
	else {
		Vector2 const f = Vector2(
			frame % size.x,
			Cast::as<float>(frame) / size.y
		) / size.toVector2();
		auto const sz = size.toVector2();
		setUV(
			f,
			f + (Vector2::RIGHT()	/ sz),
			f + (Vector2::UP()		/ sz),
			f + (Vector2::ONE()		/ sz)
		);
	}
}