# Ideas

For libraries: [`cute`](https://github.com/RandyGaul/cute_headers) & [stb](https://github.com/nothings/stb).

- For audio: [`cute_audio`](https://github.com/RandyGaul/cute_headers/blob/master/cute_sound.h) or [miniaudio](https://miniaud.io/).

- For future reference: [`stb_c_lexer`](https://github.com/nothings/stb/blob/master/stb_c_lexer.h).

## Graphical Pipeline Options

### SDL3's [GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU)

**Benefits:**
- Easy(er) option

**Problems:**
- Doesn't support old(ish) devices (at LEAST OpenGL 4.2)
- Left-handed coordinate system - engine uses right-handed
- How many features are dependent on OpenGL-specific stuff?
- How many features that are used are unavailable on SDL3?

### Custom API

**Benefits:**
- Full control over behaviour
- Can support a wide range of devices

**Problems:**
- It would be a *nightmare* to build and maintain
