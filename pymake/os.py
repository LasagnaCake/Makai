from pymake.mri import MRI_SCRIPTS
from pymake.osvar import OSDependentValue


class OSInfo:
    name: str
    mri: str
    compiler_exec: str
    linker_exec: str
    exec_type: str
    libs: list[str]

    def __init__(
        self, name: str, mri: str, exec_type: str = "", libs: list[str] = []
    ) -> None:
        self.mri = mri
        self.exec_type = exec_type
        self.libs = libs


OS = OSDependentValue[OSInfo](
    OSInfo(
        "win",
        MRI_SCRIPTS.windows,
        ".exe",
        [
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
        ],
    ),
    OSInfo("linux", MRI_SCRIPTS["linux"]),
)
