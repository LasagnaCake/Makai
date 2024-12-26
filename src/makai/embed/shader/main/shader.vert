#version 420 core

#pragma optimize(on)

#define MAX_INSTANCES 32

precision mediump float;

uniform mat4 vertMatrix;
uniform mat4 normalsMatrix;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertUV;
layout (location = 2) in vec4 vertColor;
layout (location = 3) in vec3 vertNormal;
//layout (location = 4) in uint vertFlags;

struct Transform3D {
	vec2	position;
	float	rotation;
	vec2	scale;
};

out vec3 fragCoord3D;
out vec2 fragUV;
out vec4 fragColor;
out vec3 fragNormal;

out vec2 warpUV;

//out vec3 fragLightColor;
//out vec3 fragShadeColor;

uniform vec2	uvShift	= vec2(0);

uniform Transform3D warpTrans;

// [ OBJECT INSTANCES ]
uniform vec3[MAX_INSTANCES]	instances;

vec3 getInstancePosition() {
	return (instances[gl_InstanceID]);
}

vec4 transformed(vec3 vec) {
	return vertMatrix * vec4(vec + getInstancePosition(), 1.0);
}

void main() {
	// Warping
	vec2 warp = vertUV;
	warp *= warpTrans.scale;
	warp.x = warp.x * cos(warpTrans.rotation) - warp.y * sin(warpTrans.rotation);
	warp.y = warp.x * sin(warpTrans.rotation) + warp.y * cos(warpTrans.rotation);
	warpUV = warp + warpTrans.position;
	// Vertex & Normal
	vec4 vertex	= transformed(vertPos);
	vec3 normal	= normalize(mat3(normalsMatrix) * vertNormal);
	// Point Size
	gl_PointSize = vertex.z;
	// Coordinates
	gl_Position	= vertex;
	// TODO: Proper shading
	//fragShadeColor = getShadingColor(vertex.xyz, normal);
	fragColor = vertColor;
	//fragLightColor = calculateLights(vertex.xyz, normal);
	fragUV = vertUV + uvShift;
	fragCoord3D	= vertex.xyz;
	fragNormal	= normal;
}