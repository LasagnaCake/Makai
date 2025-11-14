require('make.options')

Libraries = {}

local function Lib(include, lib)
	return {
		include	= include;
		lib		= lib;
	}
end

Libraries.SDL2		= Lib({'lib/SDL2-2.0.10/include'}, {sdl = 'lib/SDL2-2.0.10/lib/libSDL2.dll.a'; sdlNet = 'lib/SDL2-2.0.10/lib/libSDL2_net.a'})
Libraries.OpenGL	= Lib({'lib/OpenGL' + Options.glLoader + '/include'}, nil)
Libraries.STB		= Lib({'lib/stb'}, nil)
Libraries.Cute		= Lib({'lib/cute_headers'}, nil)
Libraries.CPPCodec	= Lib({'lib/cppcodec-0.2'}, nil)
Libraries.CryptoPP	= Lib({'lib/cryptopp/include'}, {cryptopp = 'lib/cryptopp/lib/libcryptopp.a'})
Libraries.XML2JSON	= Lib({'lib/xml2json/include'}, nil)
Libraries.MiniAudio = Lib({'lib/miniaudio', 'lib/minivorbis'}, nil)

Libraries.Windows = {
	'ole32',
	'oleaut32',
	'imm32',
	'winmm',
	'version',
	'powrprof',
	'comdlg32',
	'setupapi',
	'gdi32',
	'dwmapi',
	'bcrypt'
}

Library = {}

function Library.Pack(libraries)
	local t = {
		libraries = libraries
	}

	function t:includes()
		local inc = '-I"' .. Options.fullPath()
		for _, v in ipairs(self.libraries) do
			inc  = inc .. table.concat(v.include, '" -I"' .. Options.fullPath()) .. '"'
		end
		return inc
	end

	function t:libraries()
		local libs = {}
		for _, v in ipairs(self.libraries) do
			if v ~= nil then
				libs = {table.unpack(libs), table.unpack(v.libraries)}
			end
		end
		for i, v in ipairs(libs) do
			libs[i] = Options.fullPath() .. '/' .. v
		end
		return libs
	end
end