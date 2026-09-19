import os
import platform as pf

from pymake.osvar import OSDependentValue


class OSInfo:
    name: str
    arch: str
    exec_type: str
    libs: list[str]
    has_dlls: bool
    shared_lib_name: str

    def __init__(
        self,
        name: str,
        arch: str,
        exec_type: str = "",
        libs: list[str]|None = None,
        has_dlls: bool = False,
        shared_lib_name: str = ".so"
    ) -> None:
        if libs is None:
            libs = list[str]()
        self.name = name
        self.arch = arch
        self.exec_type = exec_type
        self.libs = list[str](libs)
        self.has_dlls = has_dlls
        self.shared_lib_name = shared_lib_name

    def full_name(self) -> str:
        return self.name + self.arch

    def lib_name(self, shared: bool) -> str:
        return self.shared_lib_name if shared else ".a"

ARCH = [arch.removesuffix("bit") for arch in pf.architecture()]

__WINDOWS_LIBS = [
        "ole32",
        "oleaut32",
        "imm32",
        "winmm",
        "version",
        "powrprof",
        "comdlg32",
        "setupapi",
        "gdi32",
        "dwmapi",
        "bcrypt",
        "dbghelp",
    ]

OS = OSDependentValue[OSInfo](
    OSInfo(
        "win",
        ARCH[0].removesuffix("bit"),
        ".exe",
        __WINDOWS_LIBS,
        True,
        ".dll.a"
    ),
    OSInfo("linux", ARCH[0])
)

target_os: str = "win"

def info() -> OSInfo:
    return OS[target_os]
