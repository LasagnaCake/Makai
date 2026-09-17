from typing import final

from pymake.os import info
from pymake.synchro import spawn

__os_info = info()

class Builder:
    compiler: str
    linker: str
    flags: list[str]

    class Target:
        name: str
        flags: list[str]

        def __init__(self, name: str, flags: list[str]):
            self.name = name
            self.flags = flags

    def __init__(self, compiler: str, linker: str = "ld", flags: list[str]|None = None):
        if flags is None:
            flags = []
        self.compiler = compiler
        self.linker = linker
        self.flags = list[str](flags)

    async def compile(self, target: Target, folder: str, file: str, flags: list[str]|None = None):
        if flags is None:
            flags = []
        all_flags = list[str](target.flags) + list[str](self.flags) + list[str](flags)
        _ = await spawn(
            executable=self.compiler,
            args=[f"{folder}/{file}.cpp", "-o", f"{folder.replace('/', '.')}.{file}.{target.name}.o"] + all_flags,
            check=True
        )

    async def build(self, target: Target, folder: str, file: str, flags: list[str]|None = None):
        if flags is None:
            flags = []
        all_flags = list[str](target.flags) + list[str](self.flags) + list[str](flags)
        _ = await spawn(
            executable=self.compiler,
            args=[f"{folder}/{file}.cpp", "-o", f"{folder.replace('/', '.')}.{file}.{target.name}{__os_info.exec_type}"] + all_flags,
            check=True
        )

_BASE_FLAGS: list[str] = [
    "-m64",
    "-std=gnu++20",
    "-fms-extensions"
]

_FLAGS_GCC: list[str] = [
    "-fconcepts-diagnostics-depth=4",
    "-fcoroutines",
]

_FLAGS_CLANG: list[str] = [
    "-frelaxed-template-template-args",
]

_FLAGS_OPTIMIZE: list[str] = [
    "-funswitch-loops",
    "-fpredictive-commoning",
    "-fgcse-after-reload",
    "-ftree-vectorize",
    "-fexpensive-optimizations"
]

_FLAGS_DEBUG: list[str] = [
    "-DMAKAILIB_DEBUG",
    "-DCTL_CONSOLE_OUT",
    "-DNDEBUG",
]

_FLAGS_DEBUG_EVERYTHING: list[str] = [
    "-DMAKAILIB_DEBUG_ABSOLUTELY_EVERYTHING"
]

@final
class Targets:
    DEBUG: Builder.Target = Builder.Target(
        "debug",
        _BASE_FLAGS
    )

    RELEASE: Builder.Target = Builder.Target(
        "release",
        _BASE_FLAGS
    )

@final
class Toolchain:
    @final
    class C:
        GCC: Builder = Builder("gcc", flags = [])
        CLANG: Builder = Builder("clang", flags = [])
        MINGW_GCC: Builder = Builder("mingw32-gcc", flags = [])
        MINGW_LINUX_GCC: Builder = Builder("x86_64-w64-mingw32-gcc", flags = [])

    @final
    class CPP:
        GCC: Builder = Builder("g++", flags = _FLAGS_GCC)
        CLANG: Builder = Builder("clang++", flags = _FLAGS_CLANG)
        MINGW_GCC: Builder = Builder("mingw32-g++", flags = _FLAGS_GCC)
        MINGW_LINUX_GCC: Builder = Builder("x86_64-w64-mingw32-g++", flags = _FLAGS_GCC)
