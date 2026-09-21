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
vendored.vendor_header_only("miniaudio", "miniaudio")
vendored.vendor_header_only("minivorbis", "minivorbis")
vendored.vendor_header_only("opengl", "OpenGL")
vendored.vendor_header_only("glad", "OpenGL/GLAD/include")
vendored.vendor_header_only("gl3w", "OpenGL/GL3W/include")
vendored.vendor_header_only("json2xml", "json2xml")
vendored.vendor_header_only("xml2json", "xml2json/include/xml2json")

@subtask
async def vendor_in_libraries():
	await vendored.clean_cache()
	await vendored.pack_all()
	await pipe_into(["echo", f"'{vendored.mri_script()}'"], ["ar", "-M"])
	await spawn(["ranlib", "obj/extern/extern.3p.a"])

@checkpoint
async def begin():
	await spawn(["rm", "-rf", f"{os.getcwd()}/output/*"])
	gp = Group()
	gp.spawn(["mkdir", "-p", f"{os.getcwd()}/output/lib"])
	gp.spawn(["mkdir", "-p", f"{os.getcwd()}/output/include"])
	gp.spawn(["mkdir", "-p", f"{os.getcwd()}/obj/extern"])
	await gp.await_all()

@checkpoint
async def next_step():
	pass

@checkpoint
async def end():
	await spawn(["rm", "-rf", f"{os.getcwd()}/2"])
	pass

def includes_for(subsystem: str) -> list[str]:
	ocl_include = vendored.include("ocl") if info().name == "linux" else vendored.includes("ocl", "ocl-ext", "ocl-util", "ocl-util-cpp")
	match (subsystem):
		case "impl":	return vendored.includes("stb", "cute", "glad", "miniaudio", "gl3w")
		case "audio":	return vendored.includes("miniaudio", "minivorbis")
		case "graph":	return vendored.includes("sdl", "opengl", "glad", "gl3w", "stb")
		case "core":	return vendored.include("sdl")
		case "data":	return vendored.include("cryptopp")
		case "file":	return vendored.includes("xml2json", "json2xml")
		case "image":	return vendored.includes("stb")
		case "mp":		return ocl_include
		case "net":		return vendored.includes("curl", "sdl-net", "cute")
		case "regex":	return vendored.includes("pcre2-8", "pcre2-16", "pcre2-32", "pcre2-posix")
		case "tool":	return vendored.includes("cryptopp")
		case _:			return []

@subtask
async def pack_library(target: str, lite: bool):
	objects: list[str] = [f"{os.getcwd()}/obj/{target}/{file}" for file in os.listdir(f"{os.getcwd()}/obj/{target}") if f".{target}.o" in file]
	if target != "release":
		await spawn(["ar", "rcvs", f"{os.getcwd()}/output/lib/libmakai.{target}.a"] + objects)
	else:
		await spawn(["ar", "rcvs", f"{os.getcwd()}/output/lib/libmakai.a"] + objects)
	if not lite:
		await vendored.finalize("makai", target if target != "release" else None)

@subtask
async def compile_all(compiler: str, target: str, optimize: str, subsystems: list[str]|None = None):
	tc_cpp	= Toolchain.get_for("c++", compiler, target)
	await tc_cpp.clean_cache()
	flags = Flags(
		f"-o{optimize}",
		"-I",
		f"{os.getcwd()}/src"
	)
	print("Compilation time!")
	print((flags + vendored.includes("stb", "cute", "glad", "miniaudio", "gl3w")).unpack())
	if subsystems is None:
		await join_groups(
			# TODO: makai/embed
			tc_cpp.compile_folder("makai/impl", flags + includes_for("impl")),
			tc_cpp.compile_folder("makai/audio", flags + includes_for("audio")),
			tc_cpp.compile_folder("makai/graph", flags + includes_for("graph")),
			tc_cpp.compile_folder("makai/core", flags + includes_for("core")),
			tc_cpp.compile_folder("makai/data", flags + includes_for("data")),
			tc_cpp.compile_folder("makai/file", flags + includes_for("file")),
			tc_cpp.compile_folder("makai/image", flags + includes_for("image")),
			tc_cpp.compile_folder("makai/lang", flags + includes_for("lang")),
			tc_cpp.compile_folder("makai/lexer", flags + includes_for("lexer")),
			tc_cpp.compile_folder("makai/mp", flags + includes_for("mp")),
			tc_cpp.compile_folder("makai/net", flags + includes_for("net")),
			tc_cpp.compile_folder("makai/parser", flags + includes_for("parser")),
			tc_cpp.compile_folder("makai/regex", flags + includes_for("regex")),
			tc_cpp.compile_folder("makai/tool", flags + includes_for("tool")),
			tc_cpp.compile_folder("makai/video", flags + includes_for("video"))
		).await_all()
	else:
		await join_groups(
			*[
				tc_cpp.compile_folder(f"makai/{sub}", flags + includes_for(sub.split('/')[0]))
				for sub in subs
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
	parser.add_argument("-t", "--task", action="extend", dest="tasks", nargs="+", choices=["vendor", "devmode", "debug", "release", "tools"])
	parser.add_argument("-o", default="2", choices=["g", "s", "0", "1", "2", "3"], dest="optimize")
	parser.add_argument("-mm", "--math-mode", default="fast", choices=["fast", "normal", "safe"], dest="math")
	parser.add_argument("-DT", "--debug-tooling", action="store_true", dest="debug-tools")
	parser.add_argument("-s", "--sub", "--subsystem", action="extend", dest="subsystems", default=[])
	parser.add_argument("-L", "--lite", dest="lite", action="store_true", default=False)
	parser.add_argument("-tc", "--toolchain", dest="compiler", default="gcc", choices=["gcc", "clang", "mingw-win", "mingw-linux"])
	parser.add_argument("-x", "--os", dest="os", choices=["win", "linux"], default="win")
	parser.add_argument("-S", "--sync", action="store_true", default=False, dest="sync")
	cfg = parser.parse_args(argv)
	Group.in_parallel = not cfg.sync
	osinfo.target_os = cfg.os
	print(cfg)
	await begin()
	for task in cfg.tasks:
		if task == "vendor":
			vendor_in_libraries()
		elif task == "all":
			compile_all(cfg.compiler, "devmode", "g")
			compile_all(cfg.compiler, "debug", "g")
			compile_all(cfg.compiler, "release", cfg.optimize)
		elif task == "release":
			compile_all(cfg.compiler, "release", cfg.optimize)
		else:
			compile_all(cfg.compiler, task, cfg.optimize)
	await next_step()
	for task in cfg.tasks:
		if task == "all":
			pack_library("devmode", cfg.lite)
			pack_library("debug", cfg.lite)
			pack_library("release", cfg.lite)
		elif task != "vendor":
			pack_library(task, cfg.lite)
	await next_step()
	if False:
		pass
	await end()
def run():
	asyncio.run(pymake_main())
