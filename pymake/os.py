import os
import platform as pf

from pymake.osvar import OSDependentValue


class OSInfo:
	def __init__(
		self,
		name: str,
		arch: str,
		exec_type: str = "",
		libs: list[str]|None = None,
		has_dlls: bool = False,
		shared_lib_name = ".so"
	) -> None:
		if libs is None:
			libs = list[str]()
		self.name = name
		self.arch = arch
		self.exec_type = exec_type
		self.libs = list[str](libs)
		self.has_dlls = has_dlls
		self.static_lib_name = ".a"
		self.shared_lib_name = shared_lib_name

	def full_name(self) -> str:
		return self.name + self.arch

	def lib_name(self, shared: bool) -> str:
		return self.shared_lib_name if shared else self.static_lib_name

ARCH = pf.architecture()

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
		ARCH[0],
		".exe",
		__WINDOWS_LIBS,
		True
	),
	OSInfo("linux", ARCH[0])
)

def info() -> OSInfo:
	return OS[os.name]
