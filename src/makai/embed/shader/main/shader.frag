#version 420 core

#pragma optimize(on)

precision mediump float;

in vec3 fragCoord3D;
in vec2 fragUV;
in vec4 fragColor;
in vec3 fragNormal;

in vec2 warpUV;

//in vec3 fragLightColor;
//in vec3 fragShadeColor;

layout (location = 0) out vec4	FragColor;
layout (location = 1) out vec4	FragNormal;
layout (location = 2) out vec4	FragPosition;

struct TextureEffect {
	bool		enabled;
	sampler2D 	image;
	float		alphaClip;
};

struct BlendTextureEffect {
	bool		enabled;
	sampler2D 	image;
	vec3		strength;
	uint32		equation;
};

struct NormalMapEffect {
	bool		enabled;
	sampler2D	image;
};

struct EmissionEffect {
	bool		enabled;
	sampler2D	image;
	float		strength;
};

struct WarpEffect {
	bool		enabled;
	sampler2D	image;
	uint32		channelX;
	uint32		channelY;
};

uniform vec4 albedo = vec4(1);

// [ SIMPLE SHADING ]
struct ShadingEffect {
	bool	enabled;
	float	intensity;
	vec3	direction;
};

struct AmbientData {
	vec3	color;
	float	strength;
};

struct LightEffect {
	bool	enabled;
};

// [ DISTANCE-BASED FOG ]
struct FogEffect {
	bool	enabled;
	float	start;
	float	stop;
	float	strength;
	vec4	color;
};

// [ COLOR INVERSION ]
struct NegativeEffect {
	bool	enabled;
	float	strength;
};

// [ COLOR TO GRADIENT ]
struct GradientEffect {
	bool	enabled;
	/**
	* If -1, the average between the color channels is used.
	* Else, the specified channel is used.
	* MUST be between -1 and 3.
	*/
	int		channel;
	vec4	start;
	vec4	end;
	bool	invert;
};

// [ HSLBC MODIFIERS ]
uniform float	hue			= 0;
uniform float	saturation	= 1;
uniform float	luminosity	= 1;
uniform float	brightness	= 0;
uniform float	contrast	= 1;

// [ DEBUG MODE ]
uniform uint32	debugView	= 0;

uniform AmbientData		ambient		= AmbientData(vec3(1), 1);
uniform LightEffect		lights		= LightEffect(false);
uniform ShadingEffect	shade		= ShadingEffect(false, 0.5, vec3(0, 1, 0));
uniform FogEffect		farFog		= FogEffect(false, 0, 1, 1, vec4(1));
uniform FogEffect		nearFog		= FogEffect(false, 0, 1, 1, vec4(1));
uniform NegativeEffect	negative	= NegativeEffect(false, 1);
uniform GradientEffect	gradient	= GradientEffect(false, -1, vec4(0), vec4(1), false);

uniform TextureEffect		imgTexture;
uniform BlendTextureEffect	blendTexture;
uniform NormalMapEffect		normalMap;
uniform EmissionEffect		emission;
uniform WarpEffect			warp;

vec3 calculateLights(vec3 position, vec3 normal) {
	if (!lights.enabled) return vec3(1);
	vec3 result = ambient.color * ambient.strength;
	// TODO: proper lighting
	#ifdef IMPLEMENT_LIGHTS
	#endif
	return result;
}

vec3 getShadingColor(vec3 position, vec3 normal) {
	if (!shade.enabled) return vec3(1);
	//vec3 direction = normalize(normalize(shade.direction) - position);
	vec3 direction = normalize(shade.direction);
	float factor = max(0, 1-dot(shade.direction, normal));
	return vec3(1) - shade.intensity * factor;
}

vec4 distanceGradient(vec4 start, vec4 end, float near, float far, float strength) {
	// The vector's length needs to be calculated here, otherwise it breaks
	float value = (length(fragCoord3D) - near) / (far - near);
	//float fogValue = 1.0 - exp(-fragDistance * fogStrength);
	value = clamp(value * strength, 0, 1);
	return mix(start, end, value);
}

vec4 applyNearFog(vec4 color) {
	return distanceGradient(color, vec4(nearFog.color.rgb, nearFog.color.a * color.a), nearFog.stop, nearFog.start, nearFog.strength);
}

vec4 applyFarFog(vec4 color) {
	return distanceGradient(color, vec4(farFog.color.rgb, farFog.color.a * color.a), farFog.start, farFog.stop, farFog.strength);
}

vec4 applyGradient(vec4 color) {
	float gradientValue;
	if (gradient.channel < 0)
		gradientValue = (color.r + color.g + color.b) / 3.0;
	else
		gradientValue = color[gradient.channel];
	gradientValue = clamp(gradientValue, 0, 1);
	if (gradient.invert)
		return mix(gradient.end, gradient.start, gradientValue);
	else
		return mix(gradient.start, gradient.end, gradientValue);
}

vec3 rgb2hsl(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsl2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 applyHSL(vec4 color) {
	vec3 hsl = rgb2hsl(color.xyz);
	hsl.x += mod(hue, 1);
    hsl.yz *= vec2(saturation, luminosity);
	return vec4(hsl2rgb(hsl), color.a);
}

vec4 applyBrightnessAndContrast(vec4 color) {
	vec3 res = color.rgb + brightness.xxx;
	return vec4(((res - 0.5f) * max(contrast, 0)) + 0.5f, color.a);
}

vec3 equate(vec3 a, vec3 b, uint32 mode) {
	switch (mode & 0x0Fu) {
		case 0x00: return a * b;
		case 0x01: return a / b;
		case 0x02: return a + b;
		case 0x03: return a - b;
		case 0x04: return max(a, b);
		case 0x05: return min(a, b);
		// Fallback
		default: return vec3(1);
	}
}

void main(void) {
	vec4 color = fragColor;
	vec3 normal = fragNormal;
	vec2 texelUV = fragUV;

	if (warp.enabled) {
		uint32 wcx = clamp(warp.channelX, 0u, 3u);
		uint32 wcy = clamp(warp.channelY, 0u, 3u);
		vec4 warpFac = texture(warp.image, warpUV);
		vec2 warpCoord = vec2(warpFac[wcx], warpFac[wcy]) * 2 - 1;
		texelUV = fragUV + warpCoord;
	}

	if (imgTexture.enabled)
		color = texture(imgTexture.image, texelUV) * fragColor;

	if (blendTexture.enabled) {
		bool reverse	= bool(blendTexture.equation & 0x200u);
		vec3 blend		= bool(blendTexture.equation & 0x40u) ? vec3(1) : texture(blendTexture.image, texelUV).rgb;
		vec3 source		= bool(blendTexture.equation & 0x80u) ? vec3(1) : color.rgb;
		vec3 src		= (reverse) ? blend : source;
		vec3 dst		= (reverse) ? source : blend;
		vec3 result		= vec3(0);
		switch (blendTexture.equation & 0x30u) {
			// Default equations
			case 0x00: result = equate((src),		(dst),		blendTexture.equation); break;
			// Destination-inverse equations
			case 0x10: result = equate((src),		(1 - dst),	blendTexture.equation); break;
			// Source-inverse equations
			case 0x20: result = equate((1 - src),	(dst),		blendTexture.equation); break;
			// Full-inverse equations
			case 0x30: result = equate((1 - src),	(1 - dst),	blendTexture.equation); break;
			// Fallback
			default: break;
		}
		if (bool(blendTexture.equation & 0x100u))
			result = clamp(result, vec3(0), vec3(1));
		color.rgb = mix(color.rgb, result, blendTexture.strength);
	}

	if (imgTexture.enabled && color.a <= (fragColor.a * imgTexture.alphaClip))
		discard;
	if (normalMap.enabled) {
		vec4 tn = texture(normalMap.image, texelUV);
		normal = tn.rgb * tn.a;
		normal = normalize(normal * 2.0 - 1.0 + normal);
	}

	color.xyz *= calculateLights(fragCoord3D, normal) * getShadingColor(fragCoord3D, normal);

	color *= albedo;

	color = applyHSL(color);

	color = applyBrightnessAndContrast(color);

	if (negative.enabled) {
		vec4 nc = vec4(vec3(1) - color.rgb, color.a);
		color = mix(color, nc, negative.strength);
	}

	if (gradient.enabled) color = applyGradient(color);

	if (nearFog.enabled) color = applyNearFog(color);

	if (farFog.enabled) color = applyFarFog(color);

	if (emission.enabled) {
		vec4	emitColor	= texture(emission.image, texelUV) * fragColor;
		float	emitFactor	= rgb2hsl(emitColor.rgb).z * emitColor.a;
		color.rgb = mix(color.rgb, emitColor.rgb, emitFactor * emission.strength);
	}

	FragColor		= color;
	FragNormal		= vec4(fragNormal, 1);
	FragPosition	= vec4(fragCoord3D, 1);

	if (debugView > 0) {
		switch(debugView) {
			case 1: FragColor = vec4(fragNormal / 2 + 0.5, 1); break;
			default: break;
		}
	}

	//gl_FragDepth = length(fragCoord3D);
	//DepthValue = length(fragCoord3D);

	//FragColor = vec4(fragColor.x, 1, 1.1-(fragDistance/50.0), 1) * albedo;
}
