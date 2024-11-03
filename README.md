# Makai 3D Application Engine

[![Core Library CI](https://github.com/LasagnaCake/MakaiLib/actions/workflows/core-lib.yml/badge.svg)](https://github.com/LasagnaCake/MakaiLib/actions/workflows/core-lib.yml)

## Table of contents

- [Overview](#Overview)
- [Supported Operating Systems](#Supported-Operating-Systems)
- [Requirements](#Requirements)
- - [Windows](#Windows)
- [How to Use](#How-to-Use)
- - [Note](#Note)
- [Libraries Used](#Libraries-Used)
- [Documentation](#Documentation)
- [TODO](#TODO)

## Overview

Formerly a C++20 header-only game engine.

Now, it's a C++20 3D application engine, built on top of SDL2 & OpenGL.
The game engine part is going to be refactored into a separate extension.

## Supported operating systems

- 64-bit Windows

No plans to support 32-bit systems.

## Requirements

### Windows

> [!important]
> **This library does not currently support clang/LLVM!**

> [!caution]
> **This library currently can only be compiled with optimization level `-Og` and below!**

- GCC (12+) via MinGW (MSYS (Recommended), Cygwin, TDM-GCC) 
- Make

#### Installing GCC via MSYS2

If using MSYS, all of them must be installed via pacman:

```
For toolchain (GCC): pacman -S mingw-w64-x86_64-toolchain
For make: pacman -S make

In a single line: pacman -S mingw-w64-x86_64-toolchain make
```

## How to use

For the old system, se the `legacy-system` branch.

### Building from scratch

1. Clone the repository (or just download it);
2. Enter the repository via command line (whichever has GCC);
3. Run `make all`;
4. Done! The result is located in the generated `output/` folder.

### Note

## Libraries used

Inclusion of these on your project are not required, and **strictly forbidden** (except OpenGL, which is **required**).
Since a version of (most of) them are bundled[^1], including your own version of those *will* cause issues.

| Name       | Purpose                                             | Bundled? |
|:----------:|-----------------------------------------------------|:-:|
| SDL2       | Window & Input handling                             | Yes (Lib file[^3]) |
| GL3W | OpenGL Wrangling | Yes ("Implementation" file[^6]) |
| GLAD | OpenGL Wrangling | Yes ("Implementation" file[^6]) |
| OpenGL     | Graphics backend                                    | No |
| SDL2_Mixer | Audio backend                                       | Yes (Lib file[^3]) |
| SDL2_Net   | Networking Backend                                  | Yes (Lib file[^3]) |
| stb_image  | Image loading                                       | Yes (Implementation file[^2]) |
| cute_c2    | Collision                                           | Yes (Header-only[^4], modified[^5]) |
| nlohmann::jsoncpp   | Multiple things                                     | Yes (Header-only[^4]) |
| cppcodec   | Data encoding & decoding                            | Only used internally, not required |
| CryptoPP   | Encryption, decryption, compression & decompression | Yes (Lib file[^3]) |

## Documentation

Currently VERY scarce. Some of the code is commented.

## TODO

See [TODO](docs/changes/TODO.md)

[^1]: I.E. every external library in the `lib` folder.

[^2]: Implementation part of header-only libraries. Implementations are located under `src/new/impl/`. 

[^3]: Either the contents of `.a` files, or the `.o` file associated with it.

[^4]: "Pure" header-only libraries.

[^5]: `inline`d EVERYTHING.

[^6]: A copy of the source file is located in the `src/new/impl/` folder, and is compiled when all other program parts are.
