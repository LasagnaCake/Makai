import os
import re
import subprocess as sp

from pymake import synchro
from pymake.os import info
from pymake.synchro import pipe_into, spawn


class Vendor:
    class Library:
        source: str|None
        headers: str
        unpack_to: str
        mri_name: str

        def __init__(self, source: str|None, headers: str, unpack_to: str, mri_name: str):
            self.source = source
            self.headers = headers
            self.unpack_to = unpack_to
            self.mri_name = mri_name

    def __init__(self):
        self.__vendored = dict[str, Vendor.Library]()
        self.__mri_libs = dict[str, str]()

    def vendor(self, name: str, path: str = "", filename: str = "", shared: bool = False):
        if path == "":
            path = name
        if filename == "":
            filename = "lib" + name
        if name in self.__vendored:
            return
        has_lib_file = True
        if shared and not info().has_dlls:
            has_lib_file = False
        self.__mri_libs[name] = "lib.3p." + name + ".a"
        self.__vendored[name] = Vendor.Library(
            ("lib/" + path + "/lib/" + info().full_name() + "/" + filename + info().lib_name(shared)) if has_lib_file else "",
            f"lib/{path}/include/",
            "obj/extern/" + name,
            "obj/extern/" + "lib.3p." + name,
        )
        return self.__vendored[name]

    def vendor_header_only(self, name: str, path: str = ""):
        if path == "":
            path = name + "/include/"
        if name in self.__vendored:
            return
        self.__vendored[name] = Vendor.Library(
            None,
            "lib/" + path,
            "",
            "",
        )
        return self.__vendored[name]

    def mri_script(self) -> str:
        return f"""
            create obj/extern/extern.3p.a
            {
                "\n".join(
                    [
                        "addlib " + self.mri_lib(x)
                        for x in self.__vendored
                        if self.__vendored[x].source is not None
                    ]
                )
            }
            save
            end
        """.replace("    ", "")

    def mri_lib(self, name: str) -> str:
        return self.__vendored[name].mri_name + ".a"

    def all_libraries(self) -> list[str]:
        return list(self.__vendored.keys())

    def __iter__(self):
        return iter([
            lib
            for lib in self.__vendored
            if self.__vendored[lib].source is not None
        ])

    def include(self, name: str, abspath: bool = False) -> list[str]:
        basepath = f"{os.getcwd()}/" if not abspath else ""
        return ["-I", f"{basepath}{self.__vendored[name].headers}"]

    def includes(self, *names: str) -> list[str]:
        inc = list[str]()
        for name in names:
            inc.extend(self.include(name))
        return inc

    async def clean_cache(self):
        await spawn(
            args=["rm", "-rf", "obj/extern/"]
        )
        await spawn(
            args=["mkdir", "-p", "obj/extern/"]
        )
        return self

    async def pack(self, name: str):
        _ = await spawn(
            args=["mkdir", "-p", self.__vendored[name].unpack_to]
        )
        _ = await spawn(
            args=["ar", "x", self.__vendored[name].source, "--output", self.__vendored[name].unpack_to]
        )
        sprocs = synchro.Group()
        for file in os.listdir(self.__vendored[name].unpack_to):
            sprocs.spawn(
                args=["mv", f"{self.__vendored[name].unpack_to}/{file}", f"{self.__vendored[name].unpack_to}/lib.3p.{name}.{file}"]
            )
        await sprocs.await_all()
        objects: list[str] = [f"{self.__vendored[name].unpack_to}/{fname}" for fname in os.listdir(self.__vendored[name].unpack_to) if ".o" in fname]
        print("\n  > ".join(objects))
        _ = await spawn(
            args=(["ar", "rcvs", self.mri_lib(name)] + objects)
        )
        return await spawn(args=["echo", ""])

    async def pack_all(self) -> str:
        sprocs = synchro.Group()
        for lib in self:
            sprocs.add(Vendor.pack(self, lib))
        await sprocs.await_all()
        return self.mri_script()

    async def finalize(self, name: str, target: str|None = None) -> None:
        target = f".{target}" if target is not None else ""
        MRI = f"""
            open output/lib/lib{name}{target}.a
            addlib obj/extern/extern.3p.a
            save
            end
        """
        await pipe_into(["echo", f"'{MRI}'"], ["ar", "-M"])
