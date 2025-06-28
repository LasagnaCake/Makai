# Refactor

## Simple

## Intermediate

## Complex

- [ ] Rethink old graphical pipeline
- - [ ] Add pipeline support
- - - Via `Dispatcher`
- [ ] Rethink old audio system
- - [ ] Add 3D audio support
- - - Perhaps via [miniaudio](https://miniaud.io/)?

## Very Complex

- [ ] Rewrite backend to use SDL3 (& also make it less dependent on OpenGL)
- - For the rendering pipeline: Perhaps [SDL3's GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU)?
- - - Problem: Left-handed coordinate system - engine uses right-handed
- - - Problem: How many features are dependent on OpenGL-specific stuff?
- - - Problem: How many features are unavailable on SDL3?