# Ideas

Compiler Backends: [QBE](https://c9x.me/compile/)

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
