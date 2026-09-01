#version 420 core

#pragma optimize(on)

precision mediump float;

in vec2 fragment;

layout (location = 0) out vec4 next;

uniform sampler2d previous;
uniform sampler2d current;
uniform sampler2d mask;

enum Packing {
	MV2P_NONE,
	MV2P_DELTA_ALPHA,
	MV2P_DELTA_MASKED,
	MV2P_BLOCK,
	MV2P_BLOCK_DELTA_ALPHA,
	MV2P_BLOCK_DELTA_MASKED,
};

enum Mode {
	MV2_FM_MIX,
	MV2_FM_ADD,
	MV2_FM_SUBTRACT,
	MV2_FM_MULTIPLY,
	MV2_FM_DIVIDE,
	MV2_FM_AVERAGE,
	MV2_FM_REVERSE_SUBTRACT,
	MV2_FM_REVERSE_DIVIDE,
};

uniform Packing packing;
uniform Mode mode;

vec4 apply(vec4 src, vec4 src, vec4 mask) {
    switch (mode) {
        case MV2_FM_MIX: return lerp(src, dst, mask);
        case MV2_FM_ADD: return dst + src * mask;
        case MV2_FM_SUBTRACT: return dst - src * mask;
        case MV2_FM_MULTIPLY: return lerp(dst, src * dst, mask);
        case MV2_FM_DIVIDE: return lerp(dst, src / dst, mask);
        case MV2_FM_AVERAGE: return lerp(dst, (src + dst) * 0.5, mask);
        case MV2_FM_REVERSE_SUBTRACT: return dst * mask - src;
        case MV2_FM_REVERSE_DIVIDE: return lerp(dst, dst / src, mask);
    }
}

vec4 process(vec4 dst, vec4 src, vec4 mask) {
    switch (packing) {
        case MV2P_NONE: return dst;
        case MV2P_DELTA_ALPHA:
        case MV2P_BLOCK_DELTA_ALPHA: return apply(dst, src, mask.aaaa);
        case MV2P_DELTA_MASKED:
        case MV2P_BLOCK_DELTA_MASKED: return apply(dst, src, mask);
    }
}

void main() {
    switch (packing) {
        case MV2P_NONE:
            next = sample(current, fragment);
        break;
        case MV2P_DELTA_ALPHA:
        case MV2P_BLOCK_DELTA_ALPHA:
        case MV2P_DELTA_MASKED:
        case MV2P_BLOCK_DELTA_MASKED:
            next = process(
                sample(current,     fragment),
                sample(previous,    fragment),
                sample(mask,        fragment),
            );
        break;
    }
}
