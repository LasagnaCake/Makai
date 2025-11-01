Options = {}

Options.Toolchain = {
	Auto	= 0;
	GCC		= 1;
	Clang	= 2;
	MinGW	= 3;
}

Options.MathMode = {
	Fast	= 0;
	Normal	= 1;
	Safe	= 2;
}

Options.optimize		= '3'
Options.math			= Options.MathMode.Fast
Options.compiler		= Options.Toolchain.GCC
Options.openmp			= false
Options.debugRelease	= false
Options.noBuffers		= false
Options.glLoader		= 'glad'
Options.ompThreads		= 128

Options.target			= 'all'

function Options.init()
	local vargs = dofile('../obj/config.lua')
	for k, v in pairs(vargs) do
		Options[k] = v
	end
end

function Options.fullPath()
	return io.popen("cd"):read()
end
