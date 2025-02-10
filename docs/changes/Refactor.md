# Great Refactor

## Simple

## Intermediate

- [x] Move danmaku layer `enum` values into own `enum`
- [x] Separate game-related stuff from core engine
- - Have it be an extension

## Complex

## Very Complex

- [ ] Rethink [menu](../../src/legacy/gamedata/menu.hpp) code
- - Have it also be an extension
- [x] Decouple [reference](../../src/makai/graph/gl/renderer/reference.hpp) from [renderable](../../src/makai/graph/gl/renderer/renderable.hpp)
- - So it can be used with other types
- [ ] Rewrite backend to not use SDL
- - For `SDL`: Pure Win32
- - For `SDL_Mixer`: `cute_audio`