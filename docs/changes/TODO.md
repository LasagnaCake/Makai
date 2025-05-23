# TODO

## High Priority

### Core library

- [x] Create `StaticList` class (dynamic list for move-only objects)
- [x] Fix collision detection being slow

### Extension library

- [x] Write test cases
- - [x] `APlayer`
- - [x] `BulletServer`
- - [x] `LaserServer`
- - [x] `ItemServer`
- - [x] `AEnemy`
- - [x] `ABoss`

## Low Priority

### Core library

- [ ] Generate documentation
- - Code is documented via doxygen comments, so must figure out how to generate docs from that
- [x] Fix core library CI
- [ ] Add HSL (Hue-Saturation-Luminosity) & BC (Brightness-Contrast) to buffer mask effect
- [ ] Embedded language (compiled (preferable, even if to bytecode), JIT or interpreted)
- - Could implement support for Squirrel/LUA/AngelScript
- - Or C# even, this one might be the more feasible option
- - - This one sounds enticing
- - Or Ruby via [Rice](https://ruby-rice.github.io/4.x/api/overview.html), perhaps?
- - What about [SWIG](https://www.swig.org/)?
- [ ] Bugs & Errors (see [Issues.txt](../../Issues.txt))

### Extension library

- [ ] Curvy/Bent lasers

## Future

- [ ] Replace SDL2 with SDL3 (perhaps?)
- [ ] See [ideas.md](ideas.md)
- [ ] (Maybe) replace JSON to other specification
- - For the *custom file formats*, that is
- - See [File Format Alternatives Proposal](../../docs/changes/AltFormats.md) for more informaion
- [ ] Multi-platform support:
- - [ ] Linux
- [ ] Add [QOI](https://github.com/phoboslab/qoi/blob/master/qoi.h) support
- [ ] Add [QOA](https://github.com/phoboslab/qoa/blob/master/qoa.h) support
- [ ] Create `Label` class for fancy text support
- - ( ) BBCode
- - ( ) Custom text tags
- - - See [Text Tags](../specifications/text-tags.md) for more info
- [ ] Support for "Game DLLs"
- - Having the game being separate from the executable, and stuff being loadable from a DLL
- (Maybe) reimplement save & load file dialogs ([via this, perhaps?](https://github.com/btzy/nativefiledialog-extended))
- [ ] Add OKLAB support (perhaps?)

## Other refactoring

See [Refactor](Refactor.md).

## Very far future (may never happen)

- Vulkanize graphical system (VERY DIFFICULT, might not be done)
- Maybe WebGPU instead?
