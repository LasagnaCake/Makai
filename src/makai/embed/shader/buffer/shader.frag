#version 420 core

#pragma optimize(on)

precision mediump float;

in vec2 fragUV;
in vec2 warpUV;
in vec2 maskUV;
in vec4 fragColor;
in vec2 screenScale;

layout (location = 0) out vec4	FragColor;

uniform vec2 pixelVU;
uniform vec2 screenVUSpace;

uniform vec2 resolution = vec2(0);

uniform vec4 albedo = vec4(1);
uniform vec4 accent = vec4(0);

uniform sampler2D screen;
uniform sampler2D depth;

// [ COLOR INVERSION ]
uniform bool	useNegative			= false;
uniform float	negativeStrength	= 1;

// [ ALPHA MASK ]
uniform bool		useMask		= false;
uniform sampler2D	mask;
uniform bool		invertMask	= false;
uniform vec4		maskAccent	= vec4(0);
uniform vec4		maskAlbedo	= vec4(1);

// [ COLOR TO GRADIENT ]
uniform bool	useGradient		= false;
/**
* If -1, the average between the color channels is used.
* Else, the specified channel is used.
* MUST be between -1 and 3.
*/
uniform int		gradientChannel	= -1;
uniform vec4	gradientStart	= vec4(0);
uniform vec4	gradientEnd		= vec4(1);
uniform bool	gradientInvert	= false;

// [ SCREEN WAVE ]
uniform bool useWave		= false;
uniform vec2 waveAmplitude	= vec2(1);
uniform vec2 waveFrequency	= vec2(1);
uniform vec2 waveShift		= vec2(1);
uniform uint32 waveShape	= 0;
uniform float waveLOD		= 4;

// [ SCREEN PRISM ]
uniform bool usePrism		= false;
uniform vec2 prismAmplitude	= vec2(1);
uniform vec2 prismFrequency	= vec2(1);
uniform vec2 prismShift		= vec2(1);
uniform uint32 prismShape	= 0;
uniform float prismLOD		= 4;

// [ SCREEN POLAR DISTORTION ]
uniform bool	usePolarWarp			= false;
uniform vec2	polarWarpPosition		= vec2(800, 450);
uniform vec2	polarWarpStrength		= vec2(1);
uniform float	polarWarpSize			= 200;
uniform vec4	polarWarpColor			= vec4(0,0,0.25,1);
uniform float	polarWarpTintStrength	= 1;
uniform bool	polarWarpFishEye		= true;

// [ TEXTURE WARPING ]
uniform bool		useWarp			= false;
uniform sampler2D	warpTexture;
uniform uint32		warpChannelX	=	0;
uniform uint32		warpChannelY	=	1;

// [ SCREEN RAINBOW EFFECT ]
uniform bool	useRainbow			= false;
uniform vec2	rainbowFrequency	= vec2(0);
uniform vec2	rainbowShift		= vec2(0);
uniform float	rainbowStrength		= 0;
uniform bool	rainbowAbsolute		= false;
uniform bool	rainbowPolar		= false;
uniform float	rainbowPolarShift	= 0;

// [ BLURRING EFFECT ]
uniform bool	useBlur			= false;
uniform vec2	blurStrength	= vec2(0.0005);

// [ OUTLINE EFFECT ]
uniform bool	useOutline				= false;
uniform vec2	outlineSize				= vec2(0.01);
uniform vec4	outlineColor			= vec4(1);
uniform bool	outlineRelativeAlpha	= true;

// [ NOISE EFFECT ]
uniform bool	useNoise				= false;
uniform vec2	noiseScale				= vec2(1);
uniform vec2	noiseOffset				= vec2(0);
uniform float	noiseStrength			= 1;
uniform float	noiseSeed				= 1;
uniform uint32	noiseType				= 0;
uniform uint32	noiseBlendSrcColorFunc	= 1;
uniform uint32	noiseBlendDstColorFunc	= 1;
uniform uint32	noiseBlendColorEq		= 1;
uniform uint32	noiseBlendSrcAlphaFunc	= 1;
uniform uint32	noiseBlendDstAlphaFunc	= 1;
uniform uint32	noiseBlendAlphaEq		= 1;

// [ HSLBC MODIFIERS ]
uniform float	hue			= 0;
uniform float	saturation	= 1;
uniform float	luminosity	= 1;
uniform float	brightness	= 0;
uniform float	contrast	= 1;

// [ DEBUG SETTINGS ]
uniform uint32	debugView	= 0;

#ifndef PI
#define PI 3.1415926535
#endif

#ifndef HPI
#define HPI (PI / 2.0)
#endif

#ifndef PHI
#define PHI 1.61803398874989484820459
#endif

#ifndef TAU
#define TAU (PI * 2.0)
#endif

#ifndef SQRT2
#define SQRT2 1.4142135623
#endif

vec2 getCorrectCoords(vec2 v) {
	v.y = screenVUSpace.y - (v.y * -1);
	return v * pixelVU;
}

// TODO?: www.youtube.com/watch?v=5EuYKEvugLU

vec4 hueToPastel(float hue) {
	hue *= PI;
	vec3 res = vec3(
		cos(hue),
		cos(hue + TAU * (1.0/3.0)),
		cos(hue + TAU * (2.0/3.0))
	);
	res = normalize(res * res) * SQRT2;
	return vec4(clamp(res, vec3(0), vec3(1)), 1);
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

vec4 applyGradient(vec4 color) {
	float gradientValue;
	if (gradientChannel < 0)
		gradientValue = (color.x + color.y + color.z) / 3.0;
	else
		gradientValue = color[gradientChannel];
	gradientValue = clamp(gradientValue, 0, 1);
	if (gradientInvert)
		return mix(gradientEnd, gradientStart, gradientValue);
	else
		return mix(gradientStart, gradientEnd, gradientValue);
}

vec4 applyRainbow(vec4 color, vec2 coords) {
	float fac = 0;
	if (rainbowPolar)  {
		vec2 target = (getCorrectCoords(rainbowShift) - gl_FragCoord.xy);
		fac = length(target / rainbowFrequency) + rainbowPolarShift;
	} else {
		vec2 rv = coords * rainbowFrequency + getCorrectCoords(rainbowShift);
		fac = rv.x + rv.y;
	}
	vec4 rainbow = vec4(hsl2rgb(vec3(fac, 1, 1)), 1);
	rainbow = mix(vec4(1), rainbow, rainbowStrength);
	if (rainbowAbsolute)
		rainbow.xyz *= color.xyz;
	return vec4(rainbow.xyz, color.w);
}

vec4 getPixelColor(vec2 uv) {
	if (!useBlur) return texture(screen, uv);
	return (
		texture(screen, uv) +
		texture(screen, uv + vec2(blurStrength.x, 0)) +
		texture(screen, uv - vec2(blurStrength.x, 0)) +
		texture(screen, uv + vec2(0, blurStrength.y)) +
		texture(screen, uv - vec2(0, blurStrength.y))
	) * 0.2;
}

float getOutlineValue(vec2 uv, vec2 oSize) {
	vec2 size = oSize / screenScale;
	float outline =
		texture(screen, uv + vec2(-size.x, 0)).a
	+	texture(screen, uv + vec2(0, size.y)).a
	+	texture(screen, uv + vec2(size.x, 0)).a
	+	texture(screen, uv + vec2(0, -size.y)).a
	+	texture(screen, uv + vec2(-size.x, size.y)).a
	+	texture(screen, uv + vec2(size.x, size.y)).a
	+	texture(screen, uv + vec2(-size.x, -size.y)).a
	+	texture(screen, uv + vec2(size.x, -size.y)).a;
	if (outlineRelativeAlpha) return min(outline, 1.0);
	else return (outline > 0.0) ? 1.0 : 0.0;
}

float angleTo(vec2 vec) {
	return - atan(vec.y, vec.x) + HPI;
}

vec2 angleV2(float angle) {
	return vec2(sin(angle), cos(angle));
}

vec2 normalTo(vec2 vec) {
	return angleV2(angleTo(vec));
}

vec2 polarWarp(out float pfac) {
	vec2 target = (getCorrectCoords(polarWarpPosition) - gl_FragCoord.xy);
	float distance = clamp(1 - length(target) / (polarWarpSize * pixelVU.y), 0, 1);
	pfac = distance;
	if (polarWarpFishEye)
		distance = sin(distance * PI);
	vec2 offset = normalTo(target);
	return (offset * distance) * (polarWarpStrength * 0.01);
}

float round(float v) {
	return floor(v + 0.5);
}

float ramp(float v) {
	return v - round(v);
}

float tri(float v) {
	return (abs(sin(v*0.5 + (HPI*0.5)))-abs(cos(v*0.5 + (HPI*0.5))));
}

float htri(float v) {
	return ramp(v/PI  + 2.5) * 2.0;
}

float hsin(float v) {
	return sin(ramp(v/PI  + 2.5) * PI);
}

float bin(float v, float lod) {
	return round(v * lod) / lod;
}

float simplenoise(vec2 xy) {
	return fract(sin(dot(xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float goldnoise(vec2 xy, float seed) {
	return fract(tan(distance(xy*PHI, xy)*seed));
}

float supernoise(vec2 xy, float seed) {
	return goldnoise(vec2(simplenoise(xy), simplenoise(xy.yx)), seed);
}

float pattern(float t, uint32 shape, float lod) {
	switch (shape) {
		// Square wave
		default:
		case 0x00:	return sign(sin(t));
		// Sine wave
		case 0x01:	return sin(t);
		case 0x02:	return bin(sin(t), lod);
		case 0x03:	return abs(sin(t));
		case 0x04:	return bin(abs(sin(t)), lod);
		// Triangle wave
		case 0x05:	return tri(t);
		case 0x06:	return bin(tri(t), lod);
		case 0x07:	return abs(tri(t));
		case 0x08:	return bin(abs(tri(t)), lod);
		// Half-sine wave
		case 0x09:	return hsin(t);
		case 0x0A:	return bin(hsin(t), lod);
		case 0x0B:	return abs(hsin(t));
		case 0x0C:	return bin(abs(hsin(t)), lod);
		// Half-triangle wave
		case 0x0D:	return htri(t);
		case 0x0E:	return bin(htri(t), lod);
		case 0x0F:	return abs(htri(t));
		case 0x10:	return bin(abs(htri(t)), lod);
		// Noise
		case 0x11:	return simplenoise(vec2(t, 1-t)) * 2.0 - 1.0;
		case 0x12:	return bin(simplenoise(vec2(t, 1-t)), lod) * 2.0 - 1.0;
	}
}

vec2 patternV2(vec2 t, uint32 shape, float lod) {
	return vec2(pattern(t.x, shape, lod), pattern(t.y, shape, lod));
}

vec4 applyOutline(vec4 color, vec2 uv) {
	return mix(color, outlineColor, getOutlineValue(uv, outlineSize) - color.a);
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

float rand(vec2 xy, uint32 type, float seed){
    switch (type) {
    	default:
    	// Simple Noise
    	case 0x00:	return simplenoise(xy);
    	// Gold Noise
    	case 0x01:	return goldnoise(xy, seed);
    	// Super Noise
    	case 0x02:	return supernoise(xy, seed);
    }
}


#define VBLEND_FUNC_DEFINE(T)			\
T vblendfunc(T src, T dst, uint32 func) {	\
	switch (func) {						\
		default:						\
		case 0x00: return T(0);			\
		case 0x01: return T(1);			\
		case 0x02: return src;			\
		case 0x03: return T(1) - src;	\
		case 0x04: return dst;			\
		case 0x05: return T(1) - dst;	\
	}									\
}

#define VBLEND_EQ_DEFINE(T)						\
T vblendeq(T a, T b, uint32 eq) {					\
	switch (eq) {								\
		default:								\
		case 0x00: return a + b;				\
		case 0x01: return a - b;				\
		case 0x02: return a * b;				\
		case 0x03: return a / b;				\
		case 0x04: return b - a;				\
		case 0x05: return b / a;				\
		case 0x06: return clamp(a + b, 0, 1);	\
		case 0x07: return clamp(a - b, 0, 1);	\
		case 0x08: return clamp(a * b, 0, 1);	\
		case 0x09: return clamp(a / b, 0, 1);	\
		case 0x0A: return clamp(b - a, 0, 1);	\
		case 0x0B: return clamp(b / a, 0, 1);	\
		case 0x0C: return max(a, b);			\
		case 0x0D: return min(a, b);			\
	}											\
}

#define VBLEND_DEFINE(T)															\
T vblend(T src, T dst, uint32 srcf, uint32 dstf, uint32 eq) {								\
	return vblendeq(vblendfunc(src, dst, srcf), vblendfunc(src, dst, dstf), eq);	\
}


VBLEND_FUNC_DEFINE(float)
VBLEND_FUNC_DEFINE(vec2)
VBLEND_FUNC_DEFINE(vec3)
VBLEND_FUNC_DEFINE(vec4)

VBLEND_EQ_DEFINE(float)
VBLEND_EQ_DEFINE(vec2)
VBLEND_EQ_DEFINE(vec3)
VBLEND_EQ_DEFINE(vec4)

VBLEND_DEFINE(float)
VBLEND_DEFINE(vec2)
VBLEND_DEFINE(vec3)
VBLEND_DEFINE(vec4)


vec4 blendNoise(vec4 color, float noise, uint32 srcColorFunc, uint32 dstColorFunc, uint32 colorEq, uint32 srcAlphaFunc, uint32 dstAlphaFunc, uint32 alphaEq) {
	return vec4(
		vblend(color.rgb, noise.xxx, srcColorFunc, dstColorFunc, colorEq),
		vblend(color.a, noise.x, srcAlphaFunc, dstAlphaFunc, alphaEq)
	);
}

vec4 applyNoise(vec4 color, vec2 uv) {
	vec2 nc = uv + color.xy + color.zw;
	float nv = rand(nc / noiseScale + noiseOffset, noiseType, noiseSeed);
	vec4 res = blendNoise(
		color,
		nv,
		noiseBlendSrcColorFunc,
		noiseBlendDstColorFunc,
		noiseBlendColorEq,
		noiseBlendSrcAlphaFunc,
		noiseBlendDstAlphaFunc,
		noiseBlendAlphaEq
	);
	return mix(color, res, noiseStrength);
}

void main() {
	// Screen wave
	vec2 wave = vec2(0);
	if (useWave) {
		wave = (fragUV.yx * waveFrequency) * TAU + waveShift;
		wave = patternV2(wave, waveShape, waveLOD) * (waveAmplitude * 0.1);
	}

	// Screen prismatic effect
	vec2 prism = vec2(0);
	if (usePrism) {
		prism = (fragUV * prismFrequency.yx) * TAU + prismShift.yx;
		prism = patternV2(prism, prismShape, prismLOD) * (prismAmplitude.yx * 0.1);
	}

	// Screen polar distortion
	vec2 pwarp = vec2(0);
	float pfac = 0;
	if (usePolarWarp) {
		pwarp = polarWarp(pfac);
	}

	// Screen texture warping
	vec2 warp = vec2(0);
	if (useWarp) {
		vec4 warpFac = texture(warpTexture, warpUV);
		warp = vec2(warpFac[warpChannelX], warpFac[warpChannelY]) * 2 - 1;
	}

	// Get pixel color
	vec2 screenUV = fragUV + prism + wave + warp + pwarp;
	vec4 color = (getPixelColor(screenUV) * fragColor * albedo) + accent;

	// Color inverter
	if (useNegative) {
		vec4 nc = vec4(vec3(1) - color.xyz, color.w);
		color = mix(color, nc, negativeStrength);
	}

	// Color to gradient
	if (useGradient) color = applyGradient(color);

	// Rainbow effect
	if (useRainbow) color = applyRainbow(color, screenUV);

	// Outline effect
	if (useOutline) color = applyOutline(color, screenUV);

	// Noise effect
	if (useNoise) color = applyNoise(color, screenUV);

	// Alpha mask
	if (useMask) {
		vec4 maskValue = clamp(texture(mask, maskUV) * maskAlbedo + maskAccent, 0, 1);
		if (invertMask) maskValue = vec4(1) - maskValue;
		color *= maskValue;
	}

	if (color.w <= 0) discard;

	color = applyHSL(color);

	color = applyBrightnessAndContrast(color);

	FragColor = mix(color, vec4(polarWarpColor.xyz, polarWarpColor.w * color.w), pfac * polarWarpTintStrength);

	if (debugView > 0) {
		switch(debugView) {
			case 1: FragColor = texture(depth, fragUV); break;
			default: break;
		}
	}
}
