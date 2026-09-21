import os
import re
from typing import final

from pymake.flags import Flags, concat
from pymake.os import info
from pymake.synchro import Group, join_groups, spawn


class Builder:
    compiler: str
    file_type: str
    linker: str
    flags: Flags

    class Target:
        name: str
        flags: Flags

        def __init__(self, name: str, flags: Flags):
            self.name = name
            self.flags = flags

    def __init__(self, file_type: str, compiler: str, linker: str = "ld", flags: Flags|None = None):
        if flags is None:
            flags = Flags()
        self.compiler = compiler
        self.linker = linker
        self.flags = flags.clone()
        self.file_type = file_type

    async def compile(self, target: Target, folder: str, file: str, flags: Flags|None = None, abspath: bool = False):
        if flags is None:
            flags = Flags()
        all_flags = concat(target.flags, target.flags, self.flags, flags)
        basename = folder.replace('/', '.')
        dir = folder if abspath else f"{os.getcwd()}/src/{folder}"
        #basename = basename[:basename.rfind(".")]
        base_decl = [self.compiler, "-c", f"{dir}/{file}", "-o", f"{basename}.{file}.{target.name}.o"]
        return await spawn(
            args=base_decl + all_flags.unpack()
        )

    async def build(self, target: Target, folder: str, file: str, flags: Flags|None = None, abspath: bool = False):
        if flags is None:
            flags = Flags()
        all_flags = concat(target.flags, self.flags, flags)
        basename = folder.replace('/', '.')
        dir = folder if abspath else f"{os.getcwd()}/src/{folder}"
        #basename = basename[:basename.rfind(".")]
        base_decl = [self.compiler, f"{dir}/{file}", "-o", f"obj/{target.name}/{basename}.{file}.{target.name}{info().exec_type}"]
        return await spawn(
            args=base_decl + all_flags.unpack()
        )

    def compile_folder(self, target: Target, folder: str, flags: Flags|None = None, abspath: bool = False) -> Group:
        procs: Group = Group()
        dir = folder if abspath else f"{os.getcwd()}/src/{folder}"
        print(dir)
        for entry in os.scandir(dir):
            if (entry.is_dir()):
                self.clean(target, entry.path, abspath)
                procs.group.extend(self.compile_folder(target, f"{folder}/{entry.name}", flags, abspath))
            if entry.is_file() and f".{self.file_type}" in entry.name and ".o" not in entry.name:
                procs.add(self.compile(target, folder, entry.name, flags, abspath))
        return procs

    def clean(self, target: Target, folder: str, abspath: bool = False):
        basepath = folder if not abspath else f"{os.getcwd()}/{folder}"
        for file in os.listdir(basepath):
            if f".{target.name}.o" in file:
                os.remove(f"{basepath}/{file}")

_BASE_FLAGS: Flags = Flags(
    "-m64",
    "-fms-extensions"
)

_BASE_FLAGS_CPP: Flags = Flags(
    "-std=gnu++20",
)

_FLAGS_GCC: Flags = Flags(
    "-fconcepts-diagnostics-depth=4",
    "-fcoroutines",
    "-fconcepts"
)

_FLAGS_CLANG: Flags = Flags(
    "-frelaxed-template-template-args"
)

_FLAGS_OPTIMIZE: Flags = Flags(
    "-funswitch-loops",
    "-fpredictive-commoning",
    "-fgcse-after-reload",
    "-ftree-vectorize",
    "-fexpensive-optimizations"
)

_FLAGS_DEBUG: Flags = Flags(
    "-DMAKAILIB_DEBUG",
    "-DCTL_CONSOLE_OUT",
    "-DNDEBUG"
)

_FLAGS_DEBUG_EVERYTHING: Flags = Flags(
    "-DMAKAILIB_DEBUG_ABSOLUTELY_EVERYTHING"
)

@final
class Targets:
    DEVMODE: Builder.Target = Builder.Target(
        "iddqd",
        _FLAGS_DEBUG + _FLAGS_DEBUG_EVERYTHING
    )

    DEBUG: Builder.Target = Builder.Target(
        "debug",
        _FLAGS_DEBUG
    )

    RELEASE: Builder.Target = Builder.Target(
        "release",
        _FLAGS_OPTIMIZE
    )

    @staticmethod
    def get_for(name: str):
        match name:
            case "devmode": return Targets.DEVMODE
            case "debug": return Targets.DEBUG
            case "release": return Targets.RELEASE
            case _: raise KeyError()

@final
class Toolchain:
    @final
    class C:
        GCC: Builder = Builder("c", "gcc", flags = _BASE_FLAGS)
        CLANG: Builder = Builder("c", "clang", flags = _BASE_FLAGS)
        MINGW_WINDOWS: Builder = Builder("c", "mingw32-gcc", flags = _BASE_FLAGS)
        MINGW_LINUX: Builder = Builder("c", "x86_64-w64-mingw32-gcc", flags = _BASE_FLAGS)

        @staticmethod
        def get_for(name: str):
            match name:
                case "gcc": return Toolchain.C.GCC
                case "clang": return Toolchain.C.CLANG
                case "mingw-win": return Toolchain.C.MINGW_WINDOWS
                case "mingw-linux": return Toolchain.C.MINGW_LINUX
                case _: raise KeyError()
    @final
    class CPP:
        GCC: Builder = Builder("cpp", "g++", flags = _FLAGS_GCC + _BASE_FLAGS + _BASE_FLAGS_CPP)
        CLANG: Builder = Builder("cpp", "clang++", flags = _FLAGS_CLANG + _BASE_FLAGS + _BASE_FLAGS_CPP)
        MINGW_WINDOWS: Builder = Builder("cpp", "mingw32-g++", flags = _FLAGS_GCC + _BASE_FLAGS + _BASE_FLAGS_CPP)
        MINGW_LINUX: Builder = Builder("cpp", "x86_64-w64-mingw32-g++", flags = _FLAGS_GCC + _BASE_FLAGS + _BASE_FLAGS_CPP)

        @staticmethod
        def get_for(name: str):
            match name:
                case "gcc": return Toolchain.CPP.GCC
                case "clang": return Toolchain.CPP.CLANG
                case "mingw-win": return Toolchain.CPP.MINGW_WINDOWS
                case "mingw-linux": return Toolchain.CPP.MINGW_LINUX
                case _: raise KeyError()

    builder: Builder
    target: Builder.Target

    def __init__(self, builder: Builder, target: Builder.Target):
        self.builder = builder
        self.target = target

    async def compile(self, folder: str, file: str, flags: Flags|None = None):
        return await self.builder.build(self.target, folder, file, flags)

    async def build(self, folder: str, file: str, flags: Flags|None = None):
        return await self.builder.build(self.target, folder, file, flags)

    def compile_folder(self, folder: str, flags: Flags|None = None, abspath: bool = False) -> Group:
        return self.builder.compile_folder(self.target, folder, flags, abspath)

    async def clean_cache(self):
        await spawn(["rm", "-rf", f"{os.getcwd()}/obj/{self.target.name}/*"])
        await spawn(["mkdir", "-p", f"{os.getcwd()}/obj/{self.target.name}"])
        return self

    @staticmethod
    def copy_headers(dir: str = "") -> Group:
        ops = Group()
        for top, folders, files in os.walk(dir):
            for folder in folders:
                ops.join_with(Toolchain.copy_headers(f"{dir}/{folder}"))
            for file in files:
                if ".hpp" not in file:
                    continue
                ops.spawn(args=["cp", f"{os.getcwd()}/{top}/{file}", f"{os.getcwd()}/output/include/{dir}"])
        return ops

    @staticmethod
    def get_for(lang: str, compiler: str, target: str):
        match lang:
            case "c": return Toolchain(Toolchain.C.get_for(compiler), Targets.get_for(target))
            case "c++": return Toolchain(Toolchain.CPP.get_for(compiler), Targets.get_for(target))
            case _: raise KeyError()
