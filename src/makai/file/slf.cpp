#include "slf.hpp"

namespace File = Makai::File;
using namespace Makai; using namespace Makai::SLF;
using namespace File;

constexpr String toFileExtension(ShaderType const& type) {
	switch (type) {
		default:
		case ShaderType::ST_INVALID:	return "INVALID";
		case ShaderType::ST_FRAGMENT:	return "vert";
		case ShaderType::ST_VERTEX:		return "frag";
		case ShaderType::ST_COMPUTE:	return "comp";
		case ShaderType::ST_GEOMETRY:	return "geom";
		case ShaderType::ST_TESS_CTRL:	return "tsct";
		case ShaderType::ST_TESS_EVAL:	return "tsev";
	}
	return "INVALID";
}

constexpr ShaderType fromFileExtension(String const& str) {
	if (str == "frag") return ShaderType::ST_FRAGMENT;
	if (str == "vert") return ShaderType::ST_VERTEX;
	if (str == "comp") return ShaderType::ST_COMPUTE;
	if (str == "geom") return ShaderType::ST_GEOMETRY;
	if (str == "tsct") return ShaderType::ST_TESS_CTRL;
	if (str == "tsev") return ShaderType::ST_TESS_EVAL;
	return ShaderType::ST_INVALID;
}

constexpr ShaderType fromFilePath(String const& path) {
	return fromFileExtension(
		OS::FS::fileExtension(path.lower())
	);
}

constexpr bool isValidShaderType(ShaderType const& type) {
	return type != ShaderType::ST_INVALID;
}

constexpr bool isValidShaderExtension(String const& path) {
	return isValidShaderType(fromFilePath(path));
}

SLFData Makai::SLF::parse(String const& slf, String const& srcFolder, bool const pathOnly) {
	DEBUGLN("Parsing SLF file...");
	// Get file location
	String dir = OS::FS::directoryFromPath(srcFolder);
	DEBUGLN("Directory: ", dir);
	// Parse content
	String content = slf;
	// Remove comments and empty lines
	content = Regex::replace(content, "(:[<]([\\s\\S]*?)[>]:)|(::([\\s\\S]*?)(\\n|\\r|\\r\\n))", "");
	content = Regex::replace(content, "((\\n|\\r|\\r\\n)+)", "|");
	// Initialize type specifier here
	ShaderType type = ShaderType::ST_INVALID;
	// Remove specifier for processing
	content = Regex::replace(content, "^[<](.*)[>]", "");
	// Process file
	SLFData result{dir};
	for (String shader: content.split('|')) {
		DEBUGLN("Line: ", shader);
		// If line is a type specifier, try and get it
		String tt = Regex::findFirst(shader, "^[<](.*)[>]").match;
		if (!tt.empty()) {
			type = fromFileExtension(
				Regex::replace(tt, "<|>", "")
			);
			continue;
		}
		// If type is a valid shader type, use it instead of deducing
		if (isValidShaderType(type))
			result.shaders.pushBack(ShaderEntry{shader, type});
		// Else, try and deduce it from shader file extension
		else {
			ShaderType st = fromFilePath(shader);
			if (!isValidShaderType(st)) {
				throw Error::InvalidValue(
					CTL::toString(
						"Invalid shader type for shader'",
						OS::FS::concatenate(dir, shader),
						"'!"
					),
					CTL::toString("File extension is '", OS::FS::fileExtension(shader), "'"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
			result.shaders.pushBack(ShaderEntry{shader, st});
		}
		if (!pathOnly)
			result.shaders.back().code = Makai::File::getText(result.shaders.back().path);
	}
	// Return result
	return result;
}

SLFData Makai::SLF::loadFile(String const& path, bool const pathOnly) {
	// Try and get the file
	return Makai::SLF::parse(Makai::File::loadText(path), path, pathOnly);
}

SLFData Makai::SLF::getFile(String const& path, bool const pathOnly) {
	// Try and get the file
	return Makai::SLF::parse(Makai::File::getText(path), path, pathOnly);
}
