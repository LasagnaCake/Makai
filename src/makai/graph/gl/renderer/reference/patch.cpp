#include "patch.hpp"

using namespace Makai::Graph::Ref;

#define INDEX(C, R) ((C) * columns + (R))
#define TRIANGLE_INDEX(C, R, T) INDEX((C), (R) + (T))

static void updatePlane(
	AReference::BoundRange const& triangles,
	Makai::List<Makai::Vector3>& shape,
	usize const column,
	usize const row,
	usize const columns
) {
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[0].position	= shape[INDEX(column, row)];
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[1].position	= shape[INDEX(column, row+1)];
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[2].position	= shape[INDEX(column+1, row)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[0].position	= shape[INDEX(column, row+1)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[1].position	= shape[INDEX(column+1, row)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[2].position	= shape[INDEX(column+1, row+1)];
}

static void updatePatch(
	AReference::BoundRange const& triangles,
	Makai::List<Makai::Vector3>& shape,
	usize const rows,
	usize const columns
) {
	for (usize i = 0; i < columns; ++i)
		for (usize j = 0; j < rows; ++j)
			updatePlane(triangles, shape, i, j, columns);
}

static void offsetBy(
	Makai::List<Makai::Vector3>& shape,
	Makai::Vector3 const& offset,
	usize const rows,
	usize const columns
) {
	for (usize i = 0; i < columns; ++i)
		for (usize j = 0; j < rows; ++j)
			shape[INDEX(i, j)] += offset;
}

static void buildShape(
	Makai::List<Makai::Vector3>& shape,
	Makai::Span<Makai::Vector2 const> const& sizes,
	usize const rows,
	usize const columns
) {
	Makai::Vector2 sum;
	for (usize i = 0; i < columns+1; ++i) {
		if (i > 0)
			sum.x += sizes[i-1].x;
		sum.y = 0;
		for (usize j = 0; j < rows+1; ++j) {
			if (j > 0)
				sum.y += sizes[i-1].y;
			shape[INDEX(i, j)] += sum;
		}
	}
}

void Impl::makePatch(
	AReference::BoundRange const& triangles,
	Makai::Vector3 const& offset,
	Makai::Span<Makai::Vector2 const> const& sizes,
	usize const rows,
	usize const columns
) {
	Makai::List<Makai::Vector3> shape;
	shape.resize((columns+1) * (rows+1));
	buildShape(shape, sizes, rows, columns);
	offsetBy(shape, offset, rows, columns);
	updatePatch(triangles, shape, rows, columns);
}

static void updatePlaneUVs(
	AReference::BoundRange const& triangles,
	Makai::Span<Makai::Vector2 const> const& uvs,
	usize const column,
	usize const row,
	usize const columns
) {
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[0].uv	= uvs[INDEX(column, row)];
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[1].uv	= uvs[INDEX(column, row+1)];
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[2].uv	= uvs[INDEX(column+1, row)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[0].uv	= uvs[INDEX(column, row+1)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[1].uv	= uvs[INDEX(column+1, row)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[2].uv	= uvs[INDEX(column+1, row+1)];
}

static void updatePlaneColors(
	AReference::BoundRange const& triangles,
	Makai::Span<Makai::Vector4 const> const& colors,
	usize const column,
	usize const row,
	usize const columns
) {
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[0].color	= colors[INDEX(column, row)];
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[1].color	= colors[INDEX(column, row+1)];
	triangles[TRIANGLE_INDEX(column, row, 0)].verts[2].color	= colors[INDEX(column+1, row)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[0].color	= colors[INDEX(column, row+1)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[1].color	= colors[INDEX(column+1, row)];
	triangles[TRIANGLE_INDEX(column, row, 1)].verts[2].color	= colors[INDEX(column+1, row+1)];
}

void Impl::setPatchUVs(
	AReference::BoundRange const& triangles,
	Makai::Span<Makai::Vector2 const> const& uvs,
	usize const rows,
	usize const columns
) {
	for (usize i = 0; i < columns; ++i)
		for (usize j = 0; j < rows; ++j)
			updatePlaneUVs(triangles, uvs, i, j, columns);
}

void Impl::setPatchColors(
	AReference::BoundRange const& triangles,
	Makai::Span<Makai::Vector4 const> const& colors,
	usize const rows,
	usize const columns
) {
	for (usize i = 0; i < columns; ++i)
		for (usize j = 0; j < rows; ++j)
			updatePlaneColors(triangles, colors, i, j, columns);
}