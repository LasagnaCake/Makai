import argparse as cli
import asyncio
import os
from sys import argv
from typing import cast

import pymake.os as osinfo
from pymake.builder import Toolchain
from pymake.flags import Flags
from pymake.os import info
from pymake.subtask import checkpoint, subtask
from pymake.synchro import Group, join_groups, pipe_into, spawn
from pymake.vendor import Vendor

vendored = Vendor()

# Vendored libraries

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
vendored.vendor_header_only("miniaudio", "miniaudio")
vendored.vendor_header_only("minivorbis", "minivorbis")
vendored.vendor_header_only("opengl", "OpenGL")
vendored.vendor_header_only("glad", "OpenGL/GLAD/include")
vendored.vendor_header_only("gl3w", "OpenGL/GL3W/include")
vendored.vendor_header_only("json2xml", "json2xml")
vendored.vendor_header_only("xml2json", "xml2json/include/xml2json")

# Include groups

vendored.set_include_group("impl", "stb", "cute", "glad", "miniaudio", "gl3w")
vendored.set_include_group("audio", "miniaudio", "minivorbis")
vendored.set_include_group("graph", "sdl", "opengl", "glad", "gl3w", "stb")
if info().name == "linux":
	vendored.set_include_group("mp", "ocl")
else:
	vendored.set_include_group("mp", "ocl", "ocl-ext", "ocl-util", "ocl-util-cpp")
vendored.set_include_group("net", "curl", "sdl-net", "cute")
vendored.set_include_group("regex", "pcre2-8", "pcre2-16", "pcre2-32", "pcre2-posix")
vendored.set_include_group("data", "cryptopp")
vendored.set_include_group("tool", "cryptopp")
vendored.set_include_group("file", "json2xml", "xml2json")
vendored.set_include_group("image", "stb")

# Base systems

base_systems: list[str] = [
	"impl",
	"audio",
	"graph",
	"mp",
	"net",
	"regex",
	"data",
	"tool",
	"image",
	"video",
	"parser",
	"lexer",
	"file"
]

@subtask
async def vendor_in_libraries():
	_ = await vendored.clean_cache()
	_ = await vendored.pack_all()
	_ = await pipe_into(["echo", "\n".join(vendored.mri_script())], ["ar", "-M"])
	_ = await spawn(["ranlib", "obj/extern/extern.3p.a"])

@checkpoint
async def begin():
	_ = await spawn(["rm", "-rf", f"{os.getcwd()}/output/*"])
	gp = Group()
	_ = gp.spawn(["mkdir", "-p", f"{os.getcwd()}/output/lib"])
	_ = gp.spawn(["mkdir", "-p", f"{os.getcwd()}/output/include"])
	_ = gp.spawn(["mkdir", "-p", f"{os.getcwd()}/obj/extern"])
	await gp.await_all()

@checkpoint
async def next_step():
	pass

@checkpoint
async def end():
	pass

def includes_for(subsystem: str) -> list[str]:
	return vendored.include_group(subsystem)

@subtask
async def pack_library(target: str, lite: bool):
	objects: list[str] = [f"{os.getcwd()}/obj/{target}/{file}" for file in os.listdir(f"{os.getcwd()}/obj/{target}") if f".{target}.o" in file]
	if target != "release":
		flib = f"output/lib/libmakai.{target}.a"
		_ = await spawn(["rm", "-rf", flib])
		_ = await spawn(["ar", "rcvs", flib] + objects)
	else:
		flib = f"{os.getcwd()}/output/lib/libmakai.a"
		_ = await spawn(["rm", "-rf", flib])
		_ = await spawn(["ar", "rcvs", flib] + objects)
	if not lite:
		await vendored.finalize("makai", target if target != "release" else None)

@subtask
async def compile_all(compiler: str, target: str, optimize: str, subsystems: list[str]|None = None):
	tc_cpp	= Toolchain.get_for("c++", compiler, target)
	if subsystems is None:
		_ = await tc_cpp.clean_cache()
	else:
		for sub in subsystems:
			await spawn(["rm", "-rf", f"{os.getcwd()}/obj/*{".".join(sub.split("/"))}.*.{target}.o"])
	flags = Flags(
		f"-O{optimize}",
		"-I",
		f"{os.getcwd()}/src"
	)
	print("Compilation time!")
	print((flags + vendored.includes("stb", "cute", "glad", "miniaudio", "gl3w")).unpack())
	if subsystems is None:
		await join_groups(
			*[
				tc_cpp.compile_folder(f"makai/{sub}", flags + includes_for(sub))
				for sub in base_systems
			]
		).await_all()
	else:
		sub = "/".join(sub.split("."))
		await join_groups(
			*[
				tc_cpp.compile_folder(f"makai/{sub}", flags + includes_for(sub.split('/')[0]))
				for sub in subsystems
			]
		).await_all()

@checkpoint
async def copy_libraries():
	pass

@checkpoint
async def do_jack_shit():
	pass

async def pymake_main():
	parser = cli.ArgumentParser(
		prog="pymake"
	)
	parser.add_argument("_")
	parser.add_argument("-t", "--task", action="extend", dest="tasks", nargs="+", choices=["vendor", "devmode", "debug", "release", "all", "tools"])
	parser.add_argument("-p", "--pack", action="extend", dest="packs", nargs="+", choices=["devmode", "debug", "release", "all"])
	parser.add_argument("-o", default="2", choices=["g", "s", "0", "1", "2", "3"], dest="optimize")
	parser.add_argument("-mm", "--math-mode", default="fast", choices=["fast", "normal", "safe"], dest="math")
	parser.add_argument("-DT", "--debug-tooling", action="store_true", dest="debug-tools")
	parser.add_argument("-s", "--sub", "--subsystem", action="extend", dest="subsystems", default=[])
	parser.add_argument("-L", "--lite", dest="lite", action="store_true", default=False)
	parser.add_argument("-tc", "--toolchain", dest="compiler", default="gcc", choices=["gcc", "clang", "mingw-win", "mingw-linux"])
	parser.add_argument("-x", "--os", dest="os", choices=["win", "linux"], default="win")
	parser.add_argument("-S", "--sync", action="store_true", default=False, dest="sync")
	parser.add_argument("--tools", default="none", dest="tooling", choices=["none", "devmode", "debug", "release"])
	parser.add_argument("--art-libs", default="none", dest="art_libs", choices=["none", "devmode", "debug", "release"])
	cfg = parser.parse_args(argv)
	Group.in_parallel = not cfg.sync
	osinfo.target_os = cfg.os
	print(cfg)
	await begin()
	subs: list[str]|None = None
	if cfg.subsystems is not None and len(cfg.subsystems) > 0:
		subs = cast(list[str], cfg.subsystems)
	for task in cfg.tasks:
		if task == "vendor":
			vendor_in_libraries()
		elif task == "all":
			compile_all(cfg.compiler, "devmode", "g", subs)
			compile_all(cfg.compiler, "debug", "g", subs)
			compile_all(cfg.compiler, "release", cfg.optimize, subs)
		elif task == "release":
			compile_all(cfg.compiler, "release", cfg.optimize, subs)
		else:
			compile_all(cfg.compiler, task, cfg.optimize, subs)
	await next_step()
	for pack in cfg.packs:
		if pack == "all":
			pack_library("devmode", cfg.lite, subs)
			pack_library("debug", cfg.lite, subs)
			pack_library("release", cfg.lite, subs)
		else:
			pack_library(task, cfg.lite)
	await next_step()
	if cfg.tooling != "none":
		pass
	if cfg.art_libs != "none":
		pass
	await end()
def run():
	asyncio.run(pymake_main())
