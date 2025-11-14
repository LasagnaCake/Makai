require('make.packer')
require('make.compiler')

local params = {
	src					= 'src/makai';
	extensions			= 'src/makai-ex';
	thirdPartyPrefix	= 'lib.3p';
}

Options.init()

local function compile()
	Compiler.init()
	Compiler.run(params.src)
end

local lock = false

local function copyHeaders()
	os.execute('mkdir -p output/include/makai')
	os.execute('find . -name "' .. params.src .. '/*.hpp" -exec cp --parents \\{\\} output/include/makai/ \\;')
	os.execute('cp -r -parents "' .. params.src .. '/ctl/*" output/include/makai/')
end

local function copyExtensions()
	os.execute('mkdir -p output/include/makai-ex')
	os.execute('find . -name "' .. params.extensions .. '/*.hpp" -exec cp --parents \\{\\} output/include/makai-ex/ \\;')
end

local function copyObjects(target)
	os.execute('mkdir -p obj/' .. target)
	os.execute('rm -f obj/' .. target .. '/*.embed.shader.' .. target .. '.o')
	os.execute('fnd . -name "' .. params.src .. '/*.' .. target ..'.o" -exec cp \\{\\} obj/' .. target .. '/ \\;')
end

local function doThirdPartyLibraries()
	for k, v in pairs(Libraries.SDL2.lib) do
		Packer.unpack(v, 'obj/extern/' .. k)
		Packer.addPrefix('obj/extern/' .. k, params.thirdPartyPrefix)
		Packer.repack('obj/extern/' .. k, params.thirdPartyPrefix)
	end
	for k, v in pairs(Libraries.CryptoPP.lib) do
		Packer.unpack(v, 'obj/extern/' .. k)
		Packer.addPrefix('obj/extern/' .. k, params.thirdPartyPrefix)
		Packer.repack('obj/extern/' .. k, params.thirdPartyPrefix)
	end
	os.execute('ar -M <makelib.extern.v2.mri')
	os.execute('ranlib obj/extern/extern.3p.a')
	os.execute('rm -rf obj/extern/st*')
end

local function makeLibrary(target)
	os.execute('rm -rf output/lib/libmakai.' .. target .. '.a')
	os.execute('mkdir output/lib')
	os.execute('ar rcvs output/lib/libmakai.' .. target .. '.a obj/' .. target .. '/*.debug.o')
	os.execute('ar -M <makelib.' .. target .. '.v2.mri')
	os.execute('ranlib output/lib/libmakai.' .. target .. '.a')
	os.execute('rm -rf output/lib/st*')
end

local function doDebug()
	compile()
	if !lock then
		lock = true
		copyHeaders()
		copyExtensions()
		doThirdPartyLibraries()
	end
	copyObjects('debug')
	makeLibrary('debug')
end

local function doRelease()
	compile()
	if !lock then
		lock = true
		copyHeaders()
		copyExtensions()
		doThirdPartyLibraries()
	end
	copyObjects('release')
	makeLibrary('release')
	os.execute('mv output/lib/libmakai.release.a output/lib/libmakai.a')
end

local function doAll()
	local coro = {
		coroutine.create(doDebug),
		coroutine.create(doRelease)
	}
	coroutine.resume(coro[0])
	coroutine.resume(coro[1])
	Options.target = 'all'
end

local function makePackage()
	os.execute('cd output; 7z a -tzip mingw64.zip lib include -r -mem=AES256; cd ..')
end

if Options.target == 'debug' then
	doDebug()
elseif Options.target == 'release' then
	doRelease()
elseif Options.target == 'all' then
	doAll()
elseif Options.target == 'package' then
	doAll()
	makePackage()
elseif Options.target == 'copy:headers' then
	copyHeaders()
elseif Options.target == 'copy:extensions' then
	copyExtensions()
else
	print(HELP_MESSAGE)
end