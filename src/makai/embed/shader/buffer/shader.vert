#version 420 core

#pragma optimize(on)

precision mediump float;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertUV;
layout (location = 2) in vec4 vertColor;

out vec2 fragUV;
out vec2 maskUV;
out vec4 fragColor;
out vec2 screenScale;
out vec2 screenVUSpace;

uniform vec2 uvShift	= vec2(0);

uniform vec2 resolution	= vec2(0);

uniform mat4 posMatrix;
uniform mat4 uvMatrix;

out vec2 warpUV;

uniform float warpRotate	= 0;
uniform vec2 warpScale		= vec2(1);
uniform vec2 warpOffset		= vec2(0);

// [ ALPHA MASK ]
uniform bool	relativeMask	= false;
uniform vec2	maskShift		= vec2(0);
uniform vec2	maskScale		= vec2(1);
uniform float	maskRotate		= 0;

void main()
{
	vec4 vertex = posMatrix * vec4(vertPos, 1.0);
	maskUV += maskShift;
	maskUV = vertUV * maskScale;
	maskUV.x = maskUV.x * cos(maskRotate) - maskUV.y * sin(maskRotate);
	maskUV.y = maskUV.x * sin(maskRotate) + maskUV.y * cos(maskRotate);
	if (relativeMask) 
		maskUV += uvShift;
	fragUV = ((uvMatrix * vec4(vertUV - 0.5, 0, 1.0)).xy + 0.5 + uvShift);
	fragColor = vertColor;
	gl_Position = vertex;
	screenScale = vec2(resolution.x/resolution.y, 1);
	vec2 warp = vertUV;
	warp.x = warp.x * cos(warpRotate) - warp.y * sin(warpRotate);
	warp.y = warp.x * sin(warpRotate) + warp.y * cos(warpRotate);
	warpUV = (warp * warpScale) + warpOffset;
} 