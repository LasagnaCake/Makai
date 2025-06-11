#version 420 core

#pragma optimize(on)

#define MAX_INSTANCES 32
#define MAX_BONES 64

precision mediump float;

uniform mat4 vertMatrix;
uniform mat4 normalsMatrix;

layout (location = 0) in vec3	vertPos;
layout (location = 1) in vec2	vertUV;
layout (location = 2) in vec4	vertColor;
layout (location = 3) in vec3	vertNormal;
layout (location = 4) in ivec4	boneIndices;
layout (location = 5) in vec4	boneWeights;

struct Transform3D {
	vec2	position;
	float	rotation;
	vec2	scale;
};

struct Armature {
	mat4 bones[MAX_BONES];
	uint boneCount;
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

// [ ARMATURE ]
uniform Armature armature;

void withArmatureAndTransforms(inout vec4 position, inout vec3 normal) {
	mat4 result = mat4(1);
	vec4 totalPosition = vec4(0);
	vec3 totalNormal = vec3(0);
	/*for (uint i = 0; i < 4; ++i) {
		if (boneIndices[i] == -2) break;
		if (boneIndices[i] > MAX_BONES) return;
		if (boneIndices[i] == -1 || boneIndices[i] > armature.boneCount) continue;
		vec4 localPosition = armature.bones[boneIndices[i]] * position;
        totalPosition += localPosition * boneWeights[i];
        vec3 localNormal = mat3(armature.bones[boneIndices[i]]) * normal;
        totalNormal += localNormal * boneWeights[i];
	}*/
	position = vertMatrix * (totalPosition + vec4(instances[gl_InstanceID], 0));
	normal = totalNormal;
}

void main() {
	// Warping
	vec2 warp = vertUV;
	warp *= warpTrans.scale;
	warp.x = warp.x * cos(warpTrans.rotation) - warp.y * sin(warpTrans.rotation);
	warp.y = warp.x * sin(warpTrans.rotation) + warp.y * cos(warpTrans.rotation);
	warpUV = warp + warpTrans.position;
	// Vertex & Normal
	vec4 vertex	= vec4(vertPos, 1);
	vec3 normal	= normalize(mat3(normalsMatrix) * vertNormal);
	withArmatureAndTransforms(vertex, normal);
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