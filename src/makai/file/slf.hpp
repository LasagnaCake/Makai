#ifndef MAKAILIB_FILE_SLF_H
#define MAKAILIB_FILE_SLF_H

#include "../compat/ctl.hpp"
#include "get.hpp"

/// @brief Shader Layout Format specification facilities.
namespace Makai::SLF {
	/// @brief Shader type.
	enum class ShaderType {
		ST_INVALID = -1,
		ST_FRAGMENT,
		ST_VERTEX,
		ST_COMPUTE,
		ST_GEOMETRY,
		ST_TESS_CTRL,
		ST_TESS_EVAL
	};

	/// @brief Shader.
	struct ShaderEntry {
		String		path;
		ShaderType	type;
		String		code = "";
	};

	/// @brief Shader layout.
	struct SLFData {
		String				folder;
		List<ShaderEntry>	shaders;
	};

	/// @brief Parses an SLF string.
	/// @param slf SLF string to parse.
	/// @param srcFolder Source folder of the SLF file.
	/// @param pathOnly Whether to only store the shader's path. By default, it is `true`.
	/// @return SLF data.
	/// @throw Error::InvalidValue on invalid shader type.
	SLFData parse(String const& slf, String const& srcFolder, bool const pathOnly = true);
	/// @brief Loads an SLF file from disk.
	/// @param path Path to file.
	/// @param pathOnly Whether to only store the shader's path. By default, it is `true`.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::InvalidValue on invalid shader type.
	SLFData loadFile(String const& path, bool const pathOnly = true);
	/// @brief Loads an SLF file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @param pathOnly Whether to only store the shader's path. By default, it is `true`.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::InvalidValue on invalid shader type.
	SLFData getFile(String const& path, bool const pathOnly = true);
}

namespace MkSLF = Makai::SLF;

namespace Makai::File {
	/// @brief Loads an SLF file from disk.
	/// @param path Path to file.
	/// @param pathOnly Whether to only store the shader's path. By default, it is `true`.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::InvalidValue on invalid shader type.
	inline SLF::SLFData loadSLF(String const& path)	{return SLF::loadFile(path);	}
	/// @brief Loads an SLF file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @param pathOnly Whether to only store the shader's path. By default, it is `true`.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::InvalidValue on invalid shader type.
	inline SLF::SLFData getSLF(String const& path)	{return SLF::getFile(path);		}
}

#endif // MAKAILIB_FILE_SLF
