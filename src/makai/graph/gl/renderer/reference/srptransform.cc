#include "core.hpp"

inline void srpTransform(Makai::Graph::Vertex& vtx, Makai::Matrix4x4 const& tmat, Makai::Matrix3x3 const& nmat) {
	vtx.position = (tmat * Makai::Vector4(vtx.position, 1.0f)).toVector3();
	vtx.normal = nmat * vtx.normal;
}

inline void srpTransform(Makai::Graph::Vertex& vtx, Makai::Transform3D const& trans) {
	Makai::Matrix4x4 tmat(trans);
	Makai::Matrix3x3 nmat(tmat.transposed().inverted().truncated(3, 3));
	srpTransform(vtx, tmat, nmat);
}