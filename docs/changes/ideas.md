# Ideas

For libraries: [`cute`](https://github.com/RandyGaul/cute_headers) & [stb](https://github.com/nothings/stb).

- For audio: [`cute_audio`](https://github.com/RandyGaul/cute_headers/blob/master/cute_sound.h) or [miniaudio](https://miniaud.io/).

- For future reference: [`stb_c_lexer`](https://github.com/nothings/stb/blob/master/stb_c_lexer.h).

## Graphical Pipeline Options

### SDL3's [GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU) (Vulkan-Style)

**Benefits:**
- "Easy" option
- (Potentially) makes it easier to target web (in the future, if a goal)

**Problems:**
- Requires migrating from SDL2 to SDL3
- **Left-handed coordinate system!** - engine uses right-handed
- VERY recent library (SDL3)
	- How many features are dependent on OpenGL-specific stuff?
	- How many features that are used are unavailable on SDL3?

### WebGPU Native ([WebGPU-Native](https://github.com/eliemichel/WebGPU-distribution) or [WebGPU-Cpp](https://github.com/eliemichel/WebGPU-Cpp) (more likely))

**Benefits:**
- Also an "easy" option

**Problems:**
- Very recent API
- (May or may not) require porting code to WGSL
	- Has nice features but... sintax is kinda ugly (´u\_u\`)

### Custom API

**Benefits:**
- Full control over behaviour
- Can support a wide range of devices

**Problems:**
- It would be a *nightmare* to build and maintain
