import argparse as cli
import asyncio
import os
from sys import argv

import pymake.os as osinfo
from pymake.builder import Toolchain
from pymake.flags import Flags
from pymake.os import info
from pymake.subtask import checkpoint, subtask
from pymake.synchro import Group, join_groups, pipe_into, spawn
from pymake.vendor import Vendor

vendored = Vendor()

vendored.vendor("sdl", "SDL2-2.0.10", "libSDL2", True)
vendored.vendor("sdl-net", "SDL2-2.0.10", "libSDL2_net")
vendored.vendor("cryptopp")
vendored.vendor("curl", shared=True)
vendored.vendor("pcre2-8", "pcre2")
vendored.vendor("pcre2-16", "pcre2")
vendored.vendor("pcre2-32", "pcre2")
vendored.vendor("pcre2-posix", "pcre2")
vendored.vendor("ocl", "OpenCL", "libOpenCL", True)
#vendored.vendor("openssl", filename="libssl", shared=True)
#vendored.vendor("crypto", path="openssl", shared=True)

if info().name == "win":
	vendored.vendor("ocl-ext", "OpenCL", "libOpenCLExt")
	vendored.vendor("ocl-util", "OpenCL", "libOpenCLUtils", True)
	vendored.vendor("ocl-util-cpp", "OpenCL", "libOpenCLUtilsCpp", True)

vendored.vendor_header_only("stb", "stb")
vendored.vendor_header_only("cute", "cute_headers")
vendored.vendor_header_only("miniaudio")
vendored.vendor_header_only("minivorbis")
vendored.vendor_header_only("opengl", "OpenGL")
vendored.vendor_header_only("glad", "OpenGL/GLAD")
vendored.vendor_header_only("gl3w", "OpenGL/GL3W")
vendored.vendor_header_only("json2xml")
vendored.vendor_header_only("xml2json")

@subtask
async def vendor_in_libraries():
	await (await vendored.clean_cache()).pack_all()
	(await pipe_into(["echo", f"'{vendored.mri_script()}'"], ["ar", "-M"]))
	await spawn(["ranlib", "obj/extern/extern.3p.a"])

@checkpoint
async def prepare_output():
	await spawn(["rm", "-rf", "output/*"])

@checkpoint
async def pack_library(target: str, lite: bool):
	objects: list[str] = [f"obj/{target}/{file}f" for file in os.listdir(f"obj/{target}") if ".o" in file]
	await spawn(["ar", "rcvs", f"obj/{target}/libmakai.a"] + objects)
	if target != "release":
		await spawn(["ar", "rcvs", f"obj/{target}/libmakai.{target}.a"] + objects)
	else:
		await spawn(["ar", "rcvs", f"obj/{target}/libmakai.a"] + objects)
	if not lite:
		await vendored.finalize("makai", target if target != "release" else None)

@subtask
async def compile_all(compiler: str, target: str, optimize: str):
	tc_c	= Toolchain.get_for("c", compiler, target)
	tc_cpp	= Toolchain.get_for("c++", compiler, target)
	await tc_cpp.clean_cache()
	flags = Flags(
		f"-o{optimize}",
		"-Isrc"
	)
	ocl_include = vendored.include("ocl") if info().name == "linux" else flags.clone().add(vendored.includes("ocl", "ocl-ext", "ocl-util", "ocl-util-cpp"))
	await join_groups(
		# TODO: makai/embed
		tc_c.compile_folder("makai/impl", flags.clone().add(vendored.includes("stb", "cute", "glad", "miniaudio", "gl3w"))),
		tc_cpp.compile_folder("makai/audio", flags.clone().add(vendored.includes("miniaudio", "minivorbis"))),
		tc_cpp.compile_folder("makai/graph", flags.clone().add(vendored.includes("sdl", "opengl", "glad"))),
		tc_cpp.compile_folder("makai/core", flags.clone().add(vendored.include("sdl"))),
		tc_cpp.compile_folder("makai/data", flags.clone().add(vendored.include("cryptopp"))),
		tc_cpp.compile_folder("makai/file", flags.clone().add(vendored.includes("xml2json", "json2xml"))),
		tc_cpp.compile_folder("makai/image", flags.clone().add(ocl_include)),
		tc_cpp.compile_folder("makai/lang", flags.clone()),
		tc_cpp.compile_folder("makai/lexer", flags.clone()),
		tc_cpp.compile_folder("makai/mp", flags.clone().add(ocl_include)),
		tc_cpp.compile_folder("makai/net", flags.clone().add(vendored.includes("curl", "sdl-net"))),
		tc_cpp.compile_folder("makai/parser", flags.clone()),
		tc_cpp.compile_folder("makai/regex", flags.clone().add(vendored.includes("pcre2-8", "pcre2-16", "pcre2-32", "pcre2-posix"))),
		tc_cpp.compile_folder("makai/tool", flags.clone().add(vendored.includes("cryptopp", "cppcodec"))),
		tc_cpp.compile_folder("makai/video", flags.clone().add(ocl_include))
	).await_all()
	await tc_cpp.copy_objects().await_all()

@checkpoint
async def copy_libraries():
	pass

async def pymake_main():
	parser = cli.ArgumentParser(
		prog="pymake"
	)
	parser.add_argument("exec")
	parser.add_argument("target")
	parser.add_argument("-o", default="2", choices=["g", "s", "0", "1", "2", "3"], dest="optimize")
	parser.add_argument("-mm", "--math-mode", default="fast", choices=["fast", "normal", "safe"], dest="math")
	parser.add_argument("-DT", "--debug-tooling", action="store_true", dest="debug-tools")
	parser.add_argument("-s", "--subsystems", action="extend", dest="subsystems", default=[])
	parser.add_argument("-L", "--lite", dest="lite", action="store_true", default=False)
	parser.add_argument("-tc", "--toolchain", dest="compiler", default="gcc", choices=["gcc", "clang", "mingw-win", "mingw-linux"])
	parser.add_argument("-a", "--os", dest="os", choices=["win", "linux"], default="win")
	cfg = parser.parse_args(argv)
	osinfo.target_os = cfg.os
	print(cfg)
	await prepare_output()
	vendor_in_libraries()
	if cfg.target == "all":
		compile_all(cfg.compiler, "devmode", "g")
		compile_all(cfg.compiler, "debug", "g")
		compile_all(cfg.compiler, "release", cfg.optimize)
	elif cfg.target == "release":
		compile_all(cfg.compiler, "release", cfg.optimize)
	else:
		compile_all(cfg.compiler, cfg.target, cfg.optimize)
	await pack_library(cfg.target, cfg.lite)
def run():
	asyncio.run(pymake_main())
