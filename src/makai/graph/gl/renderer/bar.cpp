#include "bar.hpp"

using namespace Makai; using namespace Makai::Graph;

void LinearBar::draw() {
	float percent = Math::clamp((value / max), 0.0f, 1.0f);
	float
		length	= percent * size.x,
		width	= size.y / 2
	;
	vertices[0]	= Vertex(Vector2(0, -width),		Vector2(0+uvShift,1) * uvScale							);
	vertices[1]	= Vertex(Vector2(0, +width),		Vector2(0+uvShift,0) * uvScale							);
	vertices[2]	= Vertex(Vector2(length, -width),	Vector2((dynamicUV ? percent : 1)+uvShift,1) * uvScale	);
	vertices[3]	= Vertex(Vector2(length, +width),	Vector2((dynamicUV ? percent : 1)+uvShift,0) * uvScale	);
	prepare();
	material.use(shader);
	display(
		vertices,
		4,
		material.culling,
		material.fill,
		DisplayMode::ODM_TRI_STRIP,
		material.instances.size()
	);
}

void RadialBar::draw() {
	update();
	prepare();
	material.use(shader);
	display(
		vertices,
		RADIAL_BAR_RESOLUTION + 2,
		material.culling,
		material.fill,
		DisplayMode::ODM_TRI_FAN,
		material.instances.size()
	);
}

void RadialBar::update() {
	// Progress bar percentage angle (in radians)
	float fraction = TAU * Math::clamp(value/max, 0.0f, 1.0f);
	// Set center to offset
	vertices[0].position = (offset.yx() * size);
	// For each surrounding vertex...
	for (usize i = 1; i < RADIAL_BAR_RESOLUTION + 2; ++i) {
		// UV fraction, Positional fraction
		float uvfrac, posfrac;
		// Get current vertex's fraction
		uvfrac = posfrac = ((float)(i-1))/(RADIAL_BAR_RESOLUTION);
		uvfrac *= TAU;
		// Multiply by progress' percentage angle
		posfrac *= fraction;
		// Center if aplicable
		if (centered) posfrac -= fraction/2.0 + PI;
		// Dynamicise UV if aplicable
		if (dynamicUV) uvfrac = posfrac;
		// Add angle offset to UV fraction
		uvfrac += uvShift;
		// Set vertex position
		vertices[i].position = (Math::angleV2(posfrac) * size);
		// Set vertex UV
		Vector2 uv = (((Math::angleV2(uvfrac + uvShift) / 2.0) + 0.5)) * uvScale;
		uv.y = 1 - uv.y;
		vertices[i].uv = (uv);
	}
}
