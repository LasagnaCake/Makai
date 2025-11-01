require('make.help')
require('make.libraries')
require('make.options')

local base = {'-m64', '-std=gnu++20', '-fconcepts-diagnostics-depth=16', '-fcoroutines', '-fms-extensions'}

local openmp = {}

if Options.openmp then
	openmp = {'-fopenmp', '-openmp'}
	if type(Options.ompThreads) == "number" then
		openmp[#openmp+1] = '-ftree-parallelize-loops=' .. Options.ompThreads
	else
		openmp[#openmp+1] = '-ftree-parallelize-loops=128'
	end
end

if Options.noBuffers then
	base[#base+1] = '-DMAKAILIB_DO_NOT_USE_BUFFERS'
end

local math = {}

if Options.math == Options.MathMode.fast then
	math[#math+1] = '-ffast-math'
	math[#math+1] = '-fsingle-precision-constant'
elseif Options.math == Options.MathMode.safe then
	math[#math+1] = '-frounding-math'
	math[#math+1] = '-fsignaling-nans'
end

local optimize = {table.unpack(openmp), table.unpack(math), '-funswitch-loops', '-fpredictive-commoning', '-fgcse-after-reload', '-ftree-vectorize', '-fexpensive-optimizations'}

local debugMode = {'-DMAKAILIB_DEBUG', '-DCTL_CONSOLE_OUT', '-DNDEBUG'}

local releaseMode = {}

if Options.debugRelease then
	releaseMode = {table.unpack(debugMode)}
else
	releaseMode[#releaseMode+1] = '-static'
	releaseMode[#releaseMode+1] = '-s'
end

local GL_LOADER_FLAG = '-DMAKAILIB_GL_LOADER=MAKAILIB_USE_'..string.upper(Options.glLoader);

local config = {
	debug	= {table.unpack(base), table.unpack(debugMode)								};
	release	= {table.unpack(base), table.unpack(releaseMode), '-O'..Options.optimize	}
}

Compiler = {}

Compiler.Mode = {
	Debug	= 0,
	Release	= 1
}

Compiler.Toolchain.GNU = {
	C			= 'gcc';
	CPP			= 'g++';
	Linker		= 'ld';
	Archiver	= 'ar';
}

Compiler.Toolchain.Clang = {
	C			= 'clang';
	CPP			= 'clang++';
	Linker		= 'ld';
	Archiver	= 'ar';
}

Compiler.Toolchain.MinGW = {
	C			= 'mingw32-gcc';
	CPP			= 'mingw32-g++';
	Linker		= 'mingw32-ld';
	Archiver	= 'mingw32-ar';
}

function Compiler.compile(compiler, file, out, includes)
	local cmd = ""
	if type(compiler) == "string" then
		cmd = compiler .. ' '
	else
		cmd = Compiler.Toolchain.GNU.CPP .. ' '
	end
	if Compiler.mode == Compiler.Mode.Debug then
		cmd = cmd .. table.concat(config.debug, ' ')
	elseif Compiler.mode == Compiler.Mode.Release then
		cmd = cmd .. table.concat(config.release, ' ')
	else
		error("Missing compilation mode!")
	end
	if type(includes) == "table" then
		cmd = cmd .. '-I"' .. table.concat(includes, '" -I"') .. '" '
	end
	cmd = cmd .. '-c "' .. file .. '" -o "' .. out .. '"'
	os.execute(cmd)
end

function Compiler.compileAll(compiler, type, files, prefix, includes)
	for _, v in ipairs(files) do
		Compiler.compile(compiler, v .. '.' .. type, prefix .. '.' .. v .. '.o', includes)
	end
end

function Compiler.run(...)
	for _, v in ipairs(...) do
		dofile(v .. '/make.lua')
	end
end

function Compiler.Recipe(compiler, type, prefix, includes)
	if compiler == nil then
		compiler = Compiler.Toolchain.GNU.CPP
	end

	local t = {
		compiler	= compiler;
		type		= type;
		prefix		= prefix;
		includes	= includes;
	}

	function t:compile(...)
		Compiler.compileAll(self.compiler, self.type, {table.unpack(...)}, self.prefix, self.includes)
	end
	
	return t;
end

function Compiler.init()
	os.execute('mkdir -p obj')
	local f = io.open(Options.fullPath() .. 'obj/config.lua', 'w+');
	if f then
		f:write('return {' .. table.concat(arg, ';') .. '}')
		f:close()
	end
end