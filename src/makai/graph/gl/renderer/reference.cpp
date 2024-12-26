#include "reference.hpp"

using namespace Makai;
using namespace Makai; using namespace Makai::Graph;

using Makai::Graph::VertexFunction;

inline void srpTransform(Vertex& vtx, Matrix4x4 const& tmat) {
	// Position
	Vector3 res = (tmat * Vector4(vtx.position, 1.0f)).toVector3();
	vtx.position = (res);
	// Normal
	res = (tmat * Vector4(vtx.normal, 1.0f)).toVector3();
	vtx.normal = (res);
}

inline void srpTransform(Vertex& vtx, Transform3D const& trans) {
	srpTransform(vtx, Matrix4x4(trans));
}

PlaneRef::PlaneRef(
	List<Triangle*> const& tris,
	Renderable& parent
): ShapeRef<2>(tris, parent) {
	// Get vertices
	this->tl	= &(tris[0]->verts[0]);
	this->tr1	= &(tris[0]->verts[1]);
	this->tr2	= &(tris[1]->verts[0]);
	this->bl1	= &(tris[0]->verts[2]);
	this->bl2	= &(tris[1]->verts[1]);
	this->br	= &(tris[1]->verts[2]);
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
Handle<PlaneRef> PlaneRef::setOrigin(
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
Handle<PlaneRef> PlaneRef::setOrigin(Transform3D const& trans) {
	Matrix4x4 tmat(trans);
	srpTransform(origin[0],	tmat);
	srpTransform(origin[1],	tmat);
	srpTransform(origin[2],	tmat);
	srpTransform(origin[3],	tmat);
	return this;
}

Handle<PlaneRef> PlaneRef::setUV(
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

Handle<PlaneRef> PlaneRef::setColor(
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

Handle<PlaneRef> PlaneRef::setColor(Vector4 const& col) {
	setColor(col, col, col, col);
	return this;
}

Handle<PlaneRef> PlaneRef::setNormal(
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

Handle<PlaneRef> PlaneRef::setNormal(
		Vector3 const& n
	) {
	setNormal(n, n, n, n);
	return this;
}

/// Sets the plane to its original state (last state set with setPosition).
Handle<IReference> PlaneRef::reset() {
	// Set origin
	*tl				= origin[0];
	*tr1	= *tr2	= origin[1];
	*bl1	= *bl2	= origin[2];
	*br				= origin[3];
	return this;
}

Handle<IReference> PlaneRef::transform() {
	onTransform();
	if (!fixed) return this;
	// Get transformation
	Transform3D self = local;
	self.scale *= (float)visible;
	Matrix4x4 tmat(self);
	// Calculate transformed vertices
	Vertex plane[4] = {origin[0], origin[1], origin[2], origin[3]};
	srpTransform(plane[0], tmat);
	srpTransform(plane[1], tmat);
	srpTransform(plane[2], tmat);
	srpTransform(plane[3], tmat);
	// Apply transformation
	*tl				= plane[0];
	*tr1	= *tr2	= plane[1];
	*bl1	= *bl2	= plane[2];
	*br				= plane[3];
	return this;
}

void PlaneRef::forEachVertex(VertexFunction const& f) {
	f(origin[0]);
	f(origin[1]);
	f(origin[2]);
	f(origin[3]);
}

void AnimatedPlaneRef::onTransform() {
	if (size.x == 0 || size.y == 0)
		setUV(0, 0, 0, 0);
	else setUV(
		(frame) / size,
		(frame + Vector2(1, 0)) / size,
		(frame + Vector2(0, 1)) / size,
		(frame + Vector2(1)) / size
	);
}

TriangleRef::TriangleRef(
	List<Triangle*> const& tris,
	Renderable& parent
): ShapeRef<1>(tris, parent) {
	// Get vertices
	this->a = &(tris[0]->verts[0]);
	this->b = &(tris[0]->verts[1]);
	this->c = &(tris[0]->verts[2]);
	// Setup trigon
	this->setOrigin(
		Vector3(-0.0, +1.0, 0.0),
		Vector3(-1.0, -1.0, 0.0),
		Vector3(+1.0, -1.0, 0.0)
	);
	this->setUV(
		Vector2(+0.5, +1.0),
		Vector2(+0.0, +0.0),
		Vector2(+1.0, +0.0)
	);
	this->setColor();
	this->setNormal(
		Vector3(+0.0, +0.0, -1.0)
	);
}

/// Sets the triangle's origin.
Handle<TriangleRef> TriangleRef::setOrigin(
	Vector3 const& aPos,
	Vector3 const& bPos,
	Vector3 const& cPos
) {
	origin[0].position = (aPos);
	origin[1].position = (bPos);
	origin[2].position = (cPos);
	return this;
}

/// Transforms the triangle's origin and normals by a given transform.
Handle<TriangleRef> TriangleRef::setOrigin(Transform3D const& trans) {
	Matrix4x4 tmat(trans);
	srpTransform(origin[0], tmat);
	srpTransform(origin[1], tmat);
	srpTransform(origin[2], tmat);
	return this;
}

Handle<TriangleRef> TriangleRef::setUV(
	Vector2 const& aUV,
	Vector2 const& bUV,
	Vector2 const& cUV
) {
	origin[0].uv = (aUV);
	origin[1].uv = (bUV);
	origin[2].uv = (cUV);
	return this;
}

Handle<TriangleRef> TriangleRef::setColor(
	Vector4 const& aCol,
	Vector4 const& bCol,
	Vector4 const& cCol
) {
	origin[0].color = (aCol);
	origin[1].color = (bCol);
	origin[2].color = (cCol);
	return this;
}

Handle<TriangleRef> TriangleRef::setColor(Vector4 const& col) {
	setColor(col, col, col);
	return this;
}

Handle<TriangleRef> TriangleRef::setNormal(
	Vector3 const& an,
	Vector3 const& bn,
	Vector3 const& cn
) {
	origin[0].normal = (an);
	origin[1].normal = (bn);
	origin[2].normal = (cn);
	return this;
}

Handle<TriangleRef> TriangleRef::setNormal(Vector3 const& n) {
	setNormal(n, n, n);
	return this;
}

/// Sets the triangle to its original state (last state set with setPosition).
Handle<IReference> TriangleRef::reset() {
	*a = origin[0];
	*b = origin[1];
	*c = origin[2];
	return this;
}

Handle<IReference> TriangleRef::transform() {
	onTransform();
	if (!fixed) return this;
	// Get transformation
	Transform3D self = local;
	self.scale *= (float)visible;
	Matrix4x4 tmat(self);
	// Calculate transformed vertices
	Vertex tri[3] = {origin[0], origin[1], origin[2]};
	srpTransform(tri[0], tmat);
	srpTransform(tri[1], tmat);
	srpTransform(tri[2], tmat);
	// Apply transformation
	*a	= tri[0];
	*b	= tri[1];
	*c	= tri[2];
	return this;
}

void TriangleRef::forEachVertex(VertexFunction const& f) {
	f(origin[0]);
	f(origin[1]);
	f(origin[2]);
}
