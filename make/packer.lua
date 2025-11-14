require('make.compiler')

Packer = {}


function Packer.unpack(from, to)
	os.execute('mkdir -p "' .. to .. '"')
	os.execute('ar x "' .. from .. '" --output "' .. to .. '"')
end

function Packer.addPrefix(target, prefix)
	os.execute('for file in '.. target ..'/*.o; do mv $$file "'.. target .. '/' .. prefix .. '.$$file"; done')
end

function Packer.repack(target, prefix)
	os.execute('ar rcvs "'.. prefix .. '.' .. target .. '.a" ' ..target ..'/*.o')
end