import os
import re
from typing import final

from pymake.os import info
from pymake.synchro import Group, spawn

__os_info = info()

class Builder:
    compiler: str
    file_type: str
    linker: str
    flags: list[str]

    class Target:
        name: str
        flags: list[str]

        def __init__(self, name: str, flags: list[str]):
            self.name = name
            self.flags = flags

    def __init__(self, file_type: str, compiler: str, linker: str = "ld", flags: list[str]|None = None):
        if flags is None:
            flags = []
        self.compiler = compiler
        self.linker = linker
        self.flags = list[str](flags)
        self.file_type = file_type

    async def compile(self, target: Target, folder: str, file: str, flags: list[str]|None = None):
        if flags is None:
            flags = []
        all_flags = list[str](target.flags) + list[str](self.flags) + list[str](flags)
        return await spawn(
            executable=self.compiler,
            args=[f"{folder}/{file}.{self.file_type}", "-o", f"{folder.replace('/', '.')}.{file}.{target.name}.o"] + all_flags,
            check=True
        )

    async def build(self, target: Target, folder: str, file: str, flags: list[str]|None = None):
        if flags is None:
            flags = []
        all_flags = list[str](target.flags) + list[str](self.flags) + list[str](flags)
        return await spawn(
            executable=self.compiler,
            args=[f"{folder}/{file}.cpp", "-o", f"{folder.replace('/', '.')}.{file}.{target.name}{__os_info.exec_type}"] + all_flags,
            check=True
        )

    def compile_folder(self, target: Target, folder: str, flags: list[str]|None = None, abspath: bool = False) -> Group:
        procs: Group = Group()
        for [dir, folders, files] in os.walk(folder if not abspath else f"{os.getcwd()}/{folder}"):
            for sub in folders:
                self.clean(target, f"{dir}/{sub}", abspath)
                procs.group.extend(self.compile_folder(target, f"{dir}/{sub}", flags, abspath))
            for file in files:
                procs.add(self.compile(target, dir, file, flags))
        return procs

    def clean(self, target: Target, folder: str, abspath: bool = False):
        for file in os.listdir(folder if not abspath else f"{os.getcwd()}/{folder}"):
            if re.match(f".*\\.{target.name}\\.o", file):
                os.remove(file)

_BASE_FLAGS: list[str] = [
    "-m64",
    "-fms-extensions"
]

_BASE_FLAGS_CPP: list[str] = [
    "-std=gnu++20",
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
        _FLAGS_DEBUG
    )

    RELEASE: Builder.Target = Builder.Target(
        "release",
        _FLAGS_OPTIMIZE
    )

@final
class Toolchain:
    @final
    class C:
        GCC: Builder = Builder("c", "gcc", flags = _BASE_FLAGS)
        CLANG: Builder = Builder("c", "clang", flags = _BASE_FLAGS)
        MINGW_GCC: Builder = Builder("c", "mingw32-gcc", flags = _BASE_FLAGS)
        MINGW_LINUX_GCC: Builder = Builder("c", "x86_64-w64-mingw32-gcc", flags = _BASE_FLAGS)

    @final
    class CPP:
        GCC: Builder = Builder("cpp", "g++", flags = _FLAGS_GCC + _BASE_FLAGS + _BASE_FLAGS_CPP)
        CLANG: Builder = Builder("cpp", "clang++", flags = _FLAGS_CLANG + _BASE_FLAGS + _BASE_FLAGS_CPP)
        MINGW_GCC: Builder = Builder("cpp", "mingw32-g++", flags = _FLAGS_GCC + _BASE_FLAGS + _BASE_FLAGS_CPP)
        MINGW_LINUX_GCC: Builder = Builder("cpp", "x86_64-w64-mingw32-g++", flags = _FLAGS_GCC + _BASE_FLAGS + _BASE_FLAGS_CPP)
