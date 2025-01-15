# TODO

## Current
- [ ] Generate documentation
- - Code is documented via doxygen comments, so must figure out how to generate docs from that
- [ ] Fix core library CI
- - Problem: `gcc`/`g++` is missing, somehow???
- [ ] Add HSL (Hue-Saturation-Luminosity) & BC (Brightness-Contrast) to buffer mask effect
- [ ] Embedded language (compiled (preferable, even if to bytecode), JIT or interpreted)
- - Could implement support for Squirrel/LUA/AngelScript
- - Or C# even, this one might be the more feasible option
- - - This one sounds enticing
- - What about [SWIG](https://www.swig.org/)?
- [ ] Bugs & Errors (see [Issues.txt](../../Issues.txt))

## Game extension stuff
- [ ] Curvy/Bent lasers
- [ ] File format implementation (These need better names):
- - For more info, see the [specifications](../specifications) folder
- - [ ] Animation keyframes definition
- - [ ] Danmaku stage layout definition
- - - Possibly via combining 2nd & 3rd implementations
- [ ] Other game types
- [ ] More "multi-purpose" game classes

## Future

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

## Other refactoring

See [Refactor](Refactor.md).

## Very far future (may never happen)

- Vulkanize graphical system (VERY DIFFICULT, might not be done)
- Maybe WebGPU instead?
