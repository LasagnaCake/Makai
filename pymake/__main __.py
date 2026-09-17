from asyncio.threads import to_thread

from pymake.os import info
from pymake.synchro import block_until_done as bud
from pymake.vendor import Vendor

vendored = Vendor()

vendored.vendor("sdl", "SDL2-2.0.10", "libSDL2", True)
vendored.vendor("sdl-net", "SDL2-2.0.10", "libSDL2_net")
vendored.vendor("cryptopp")
vendored.vendor("curl", shared = True)
vendored.vendor("pcre2-8", "pcre2")
vendored.vendor("pcre2-16", "pcre2")
vendored.vendor("pcre2-32", "pcre2")
vendored.vendor("pcre2-posix", "pcre2")
vendored.vendor("ocl", "OpenCL", "libOpenCL", True)

if info().name == "win":
	vendored.vendor("ocl-ext", "OpenCL", "libOpenCLExt")
	vendored.vendor("ocl-util", "OpenCL", "libOpenCLUtils", True)
	vendored.vendor("ocl-util-cpp", "OpenCL", "libOpenCLUtilsCpp", True)

async def main():
	await vendored.pack_all()

bud(main)
