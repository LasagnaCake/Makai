#include "glapiloader.cc"
#include <GL/gl.h>

#define GET_GL_POINTER(start, offset) (void*)((start) + (offset) * sizeof(float))
#define GET_GL_OFFSET(offset) (void*)((offset) * sizeof(float))

#include "vertex.hpp"

using namespace Makai; using namespace Makai::Graph;

VertexMap Vertex::defaultMap() {
	using namespace Literals::Text;
	return VertexMap({
		{"x", 0},
		{"y", 0},
		{"z", 0},
		{"u", 0},
		{"v", 0},
		{"r", 1},
		{"g", 1},
		{"b", 1},
		{"a", 1},
		{"nx", 0},
		{"ny", 0},
		{"nz", 0},
		{"b0", BONE_DEFAULT_ID},
		{"b1", BONE_DEFAULT_ID},
		{"b2", BONE_DEFAULT_ID},
		{"b3", BONE_DEFAULT_ID},
		{"w0", 0},
		{"w1", 0},
		{"w2", 0},
		{"w3", 0}
	});
}

inline float getValue(VertexMap const& vmap, String const& value, float const fallback) {
	return vmap.contains(value) ? vmap.at(value) : fallback;
}

Vertex::Vertex(VertexMap const& vmap)
	: Vertex(
		getValue(vmap, "x",		0),
		getValue(vmap, "y",		0),
		getValue(vmap, "z",		0),
		getValue(vmap, "u",		0),
		getValue(vmap, "v",		0),
		getValue(vmap, "r",		1),
		getValue(vmap, "g",		1),
		getValue(vmap, "b",		1),
		getValue(vmap, "a",		1),
		getValue(vmap, "nx",	0),
		getValue(vmap, "ny",	0),
		getValue(vmap, "nz",	0),
		getValue(vmap, "b0",	BONE_DEFAULT_ID),
		getValue(vmap, "b1",	BONE_DEFAULT_ID),
		getValue(vmap, "b2",	BONE_DEFAULT_ID),
		getValue(vmap, "b3",	BONE_DEFAULT_ID),
		getValue(vmap, "w0",	0),
		getValue(vmap, "w1",	0),
		getValue(vmap, "w2",	0),
		getValue(vmap, "w3",	0)
	)
{}

void Vertex::setAttributes() {
	// Position
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		GET_GL_OFFSET(0)
	);
	// UV
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		GET_GL_OFFSET(3)
	);
	// Color
	glVertexAttribPointer(
		2,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		GET_GL_OFFSET(5)
	);
	// Normal
	glVertexAttribPointer(
		3,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		GET_GL_OFFSET(9)
	);
	// Bone
	glVertexAttribPointer(
		4,
		4,
		GL_INT,
		GL_FALSE,
		sizeof(Vertex),
		GET_GL_OFFSET(12)
	);
	// Weight
	glVertexAttribPointer(
		5,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		GET_GL_OFFSET(16)
	);
}

void Vertex::enableAttributes() {
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
}

void Vertex::disableAttributes() {
	glDisableVertexAttribArray(5);
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}
