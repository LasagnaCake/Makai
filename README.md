# Makai: A C++20 3D (+"2D") application & game framework library

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

A C++20 3D application & game framework (with support for "Pseudo-2D"),
built on top of SDL2 & OpenGL.

## Supported operating systems

- 64-bit Windows

No plans to support 32-bit systems.

## Requirements

### Windows

> [!important]
> **This library does not currently support clang/LLVM!**

- GCC (13+) via MinGW (MSYS (Recommended), Cygwin, TDM-GCC) 
- Make

#### Optional packages

- 7-zip: Only required for packing library releases.

#### Installing via MSYS2

If using MSYS, all of them must be installed via pacman:

```
For toolchain (GCC): pacman -S mingw-w64-x86_64-toolchain
For make: pacman -S make

(Optionals)
For 7-zip: mingw-w64-x86_64-7zip

In a single line: pacman -S mingw-w64-x86_64-toolchain make mingw-w64-x86_64-7zip
```

## How to use

For the old system, se the `legacy-system` branch.

### Building from scratch

1. Clone the repository (or just download it);
2. Enter the repository via command line (whichever has GCC);
3. Run `make it`;
  - If you'd like to not pack in the non-header-only libraries together, you can passs in `lite=1` as an argument. 
4. Done! The result is located in the generated `output/` folder.

> [!note]
> If `../../../lib/xml2json/[...]: warning: floating constant exceeds range of 'float' [-Woverflow]` appears while compiling for release, ignore it - it's a third-party library issue.

### Note

## Libraries used

In older versions of the framework, these libraries came bundled inside the main ones.

Since version 2.0, there is a version of Makai that comes without some libraries (those that are not "header-only"). Those that are not included in this "lite" version are marked in the table below with an asterisk(\*).

Inclusion of these on your project are not required, and **strictly forbidden** (except OpenGL, which is **required**).
Since a version of (most of) them are bundled[^1], including your own version of those *will* cause issues.

| Name       | Purpose                                             | Bundled?                                            |
|:----------:|-----------------------------------------------------|:---------------------------------------------------:|
| SDL2       | Window & Input handling                             | \*Yes (Lib file[^3])                                |
| GL3W       | OpenGL Wrangling                                    | Yes ("Implementation" file[^6])                     |
| GLAD       | OpenGL Wrangling                                    | Yes ("Implementation" file[^6])                     |
| OpenGL     | Graphics backend                                    | No                                                  |
| miniaudio  | Audio backend                                       | Yes (Implementation file[^2])                       |
| SDL2_Net   | TCP/UDP backend                                     | \*Yes (Lib file[^3])                                |
| cURL       | Networking backend                                  | \*Yes (Lib file[^3])                                |
| stb_image  | Image loading                                       | Yes (Implementation file[^2])                       |
| xml2json   | XML-to-JSON conversion                              | Modified version only used internally, not required |
| json2xml   | JSON-to-XML conversion                              | Modified version only used internally, not required |
| CryptoPP   | Encryption, decryption, compression & decompression | \*Yes (Lib file[^3])                                |

\* Only bundled in the "full" library package, not the "lite" version.

## Documentation

The API is documented. Should be able to be generated via doxygen (TODO: figure out that).

In the meantime, you can read the headers directly.

## Bugs & issues

see [Issues](Issues.txt).

## TODO

See [TODO](docs/changes/TODO.md).

[^1]: I.E. specified bundled libraries.

[^2]: Implementation part of header-only libraries. Implementations are located under `src/new/impl/`. 

[^3]: Either the contents of `.a` files, or the `.o` file associated with it.

[^4]: "Pure" header-only libraries.

[^6]: A copy of the source file is located in the `src/new/impl/` folder, and is compiled when all other program parts are.
