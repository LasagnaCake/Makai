import os
import re
import subprocess as sp

from pymake import synchro
from pymake.mri import MRI_BASE_SCRIPT
from pymake.os import info
from pymake.synchro import spawn


class Vendor:
	class Library:
		def __init__(self, source: str, unpack_to: str, mri_name: str):
			self.source = source
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
		if shared and not info().has_dlls:
			return
		self.__mri_libs[name] = "lib.3p." + name + ".a"
		self.__vendored[name] = Vendor.Library(
			"lib/" + path + "/lib/" + info().full_name() + "/" + filename + info().lib_name(shared),
			"obj/extern/" + name,
			"obj/extern/" + "lib.3p." + name,
		)

	def mri_script(self) -> str:
		return re.sub(
			"\\$LIBRARIES",
			MRI_BASE_SCRIPT,
			"".join(
				[
					"addlib obj/extern/" + self.mri_lib(x)
					for x in self.__vendored
				]
			)
		)

	def mri_lib(self, name: str) -> str:
		return self.__vendored[name].mri_name + ".a"

	def keys(self) -> list[str]:
		return list(self.__vendored.keys())

	def __iter__(self):
		return iter(self.__vendored)

	async def pack(self, name: str):
		await spawn(
			sp.run,
			executable="mkdir",
			args=["-p", self.__vendored[name].unpack_to],
			check=True
		)
		await spawn(
			sp.run,
			executable="ar",
			args=["x", self.__vendored[name].source, "--output", self.__vendored[name].unpack_to],
			check=True
		)
		sprocs = synchro.Group()
		for file in os.listdir(self.__vendored[name].unpack_to):
			sprocs.spawn(
				lambda file: sp.run(
						executable="mv",
						args=[f"${file}", f"{self.__vendored[name].mri_name}.${file}.a"],
						check=True
					),
				file
			)
			await sprocs.all()
		await spawn(
			sp.run,
			executable="ar",
			args=["rcvs", self.mri_lib(name), f"{self.__vendored[name].unpack_to}/*.o*"],
			check=True
		)

	async def pack_all(self) -> str:
		sprocs = synchro.Group()
		for lib in self:
			sprocs.spawn(Vendor.pack, self, lib)
		await sprocs.all()
		return self.mri_script()
